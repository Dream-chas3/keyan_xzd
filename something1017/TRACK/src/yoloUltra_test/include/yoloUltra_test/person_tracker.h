#ifndef SAFINA_PERSON_TRACKER_H
#define SAFINA_PERSON_TRACKER_H

// Lightweight multi-person tracker with stable IDs and a guarded primary
// target.
//
// The primary target gets a stricter association gate:
//   - predicted centre (constant-velocity extrapolation),
//   - allowed centre distance,
//   - IoU with the previous box,
//   - box-height consistency.
// A passer-by at a very different distance/position cannot take over the
// primary track during a brief occlusion; the primary track simply stays
// unmatched (the caller holds position) until it reappears or the loss timeout
// expires.
//
// No appearance (ReID) model is used. Two people crossing at the same distance
// can still be ambiguous.

#include <opencv2/core.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

struct PersonDetection
{
    cv::Rect box;
    float confidence = 0.0f;
};

struct PersonTrack
{
    int id = -1;
    cv::Rect box;
    float confidence = 0.0f;
    double first_seen = 0.0;
    double last_seen = 0.0;
    bool matched_this_frame = false;
    bool created_this_frame = false;

    cv::Point2f center{0.0f, 0.0f};
    cv::Point2f velocity{0.0f, 0.0f};
    bool has_velocity = false;
};

class PersonTracker
{
public:
    struct Params
    {
        double new_track_confidence = 0.50;
        double match_confidence = 0.20;

        // Generic association.
        double assoc_iou_threshold = 0.30;
        double assoc_center_ratio = 0.08;   // fraction of image diagonal
        double assoc_size_tol = 0.60;       // box-height ratio tolerance

        // A track is deleted after this long without a match.
        double track_loss_timeout_s = 1.50;
        int max_tracks = 20;

        // Primary target gate.
        double primary_iou_min = 0.15;
        double primary_center_ratio = 0.10; // fraction of image diagonal
        double primary_size_tol = 0.35;     // box-height ratio tolerance
        double primary_predict_s = 0.20;    // velocity prediction horizon
        double max_predict_speed_ratio = 0.50; // diag/s, velocity clamp
    };

    PersonTracker() = default;
    explicit PersonTracker(const Params &params) : params_(params) {}

    void setParams(const Params &params) { params_ = params; }
    const Params &params() const { return params_; }

    void setPrimary(int id) { primary_id_ = id; }
    int primaryId() const { return primary_id_; }

    void reset()
    {
        tracks_.clear();
        next_id_ = 1;
        primary_id_ = -1;
    }

    const std::vector<PersonTrack> &update(const std::vector<PersonDetection> &detections,
                                           double now_s,
                                           int image_width,
                                           int image_height)
    {
        for (auto &track : tracks_)
        {
            track.matched_this_frame = false;
            track.created_this_frame = false;
        }

        const double diagonal = std::sqrt(static_cast<double>(image_width) * image_width +
                                          static_cast<double>(image_height) * image_height);

        std::vector<bool> det_used(detections.size(), false);
        std::vector<bool> track_used(tracks_.size(), false);

        int primary_index = -1;
        for (int t = 0; t < static_cast<int>(tracks_.size()); ++t)
        {
            if (tracks_[t].id == primary_id_)
            {
                primary_index = t;
                break;
            }
        }

        // ---- primary target first, with a strict gate ----
        int primary_detection = -1;
        if (primary_index >= 0)
        {
            const PersonTrack &pt = tracks_[primary_index];
            const double dt = std::max(0.0, now_s - pt.last_seen);
            const double horizon = std::min(dt, params_.primary_predict_s);
            const cv::Point2f predicted = pt.center + pt.velocity * static_cast<float>(horizon);

            double best_score = -std::numeric_limits<double>::infinity();
            for (int d = 0; d < static_cast<int>(detections.size()); ++d)
            {
                if (detections[d].confidence < params_.match_confidence)
                    continue;

                const double iou = iouOf(detections[d].box, pt.box);
                const double dist = cv::norm(centerOf(detections[d].box) - predicted);
                const bool center_ok = dist <= params_.primary_center_ratio * diagonal;
                const bool iou_ok = iou >= params_.primary_iou_min;
                if (!center_ok && !iou_ok)
                    continue;

                const double height_ratio =
                    static_cast<double>(detections[d].box.height) /
                    std::max(1, pt.box.height);
                if (height_ratio < 1.0 - params_.primary_size_tol ||
                    height_ratio > 1.0 + params_.primary_size_tol)
                    continue;

                const double score = iou - 0.1 * dist / std::max(1.0, diagonal);
                if (primary_detection < 0 || score > best_score)
                {
                    primary_detection = d;
                    best_score = score;
                }
            }

            if (primary_detection >= 0)
            {
                det_used[primary_detection] = true;
                track_used[primary_index] = true;
                applyMatch(tracks_[primary_index], detections[primary_detection], now_s, diagonal);
            }
        }

        // ---- generic association for everyone else ----
        struct Pair
        {
            int det;
            int track;
            double iou;
            double center_dist;
            bool via_iou;
        };
        std::vector<Pair> pairs;

        for (int d = 0; d < static_cast<int>(detections.size()); ++d)
        {
            if (det_used[d] || detections[d].confidence < params_.match_confidence)
                continue;

            for (int t = 0; t < static_cast<int>(tracks_.size()); ++t)
            {
                // The primary is handled only by its strict gate; never let a
                // generic pair take it over.
                if (track_used[t] || t == primary_index)
                    continue;

                const double height_ratio =
                    static_cast<double>(detections[d].box.height) /
                    std::max(1, tracks_[t].box.height);
                if (height_ratio < 1.0 - params_.assoc_size_tol ||
                    height_ratio > 1.0 + params_.assoc_size_tol)
                    continue;

                const double iou = iouOf(detections[d].box, tracks_[t].box);
                const double dist = centerDistance(detections[d].box, tracks_[t].box);
                const bool via_iou = iou >= params_.assoc_iou_threshold;
                const bool via_center = dist <= params_.assoc_center_ratio * diagonal;
                if (via_iou || via_center)
                    pairs.push_back({d, t, iou, dist, via_iou});
            }
        }

        std::sort(pairs.begin(), pairs.end(), [](const Pair &a, const Pair &b) {
            if (a.via_iou != b.via_iou)
                return a.via_iou > b.via_iou;
            if (a.via_iou)
                return a.iou > b.iou;
            return a.center_dist < b.center_dist;
        });

        for (const Pair &pair : pairs)
        {
            if (det_used[pair.det] || track_used[pair.track])
                continue;
            det_used[pair.det] = true;
            track_used[pair.track] = true;
            applyMatch(tracks_[pair.track], detections[pair.det], now_s, diagonal);
        }

        // ---- new tracks for unmatched detections ----
        std::vector<int> new_detections;
        for (int d = 0; d < static_cast<int>(detections.size()); ++d)
        {
            if (!det_used[d] && detections[d].confidence >= params_.new_track_confidence)
                new_detections.push_back(d);
        }
        std::sort(new_detections.begin(), new_detections.end(), [&](int a, int b) {
            return detections[a].confidence > detections[b].confidence;
        });

        for (int d : new_detections)
        {
            if (static_cast<int>(tracks_.size()) >= params_.max_tracks)
                break;

            PersonTrack track;
            track.id = next_id_++;
            track.box = detections[d].box;
            track.confidence = detections[d].confidence;
            track.first_seen = now_s;
            track.last_seen = now_s;
            track.matched_this_frame = true;
            track.created_this_frame = true;
            track.center = centerOf(detections[d].box);
            track.velocity = cv::Point2f(0.0f, 0.0f);
            track.has_velocity = false;
            tracks_.push_back(track);
        }

        // ---- remove tracks that have been missing too long ----
        tracks_.erase(std::remove_if(tracks_.begin(), tracks_.end(),
                                     [&](const PersonTrack &track) {
                                         return (now_s - track.last_seen) >
                                                params_.track_loss_timeout_s;
                                     }),
                      tracks_.end());

        return tracks_;
    }

    const std::vector<PersonTrack> &tracks() const { return tracks_; }
    int nextId() const { return next_id_; }

    PersonTrack *find(int id)
    {
        for (auto &track : tracks_)
            if (track.id == id)
                return &track;
        return nullptr;
    }

    const PersonTrack *find(int id) const
    {
        for (const auto &track : tracks_)
            if (track.id == id)
                return &track;
        return nullptr;
    }

    static double iouOf(const cv::Rect &a, const cv::Rect &b)
    {
        const int x1 = std::max(a.x, b.x);
        const int y1 = std::max(a.y, b.y);
        const int x2 = std::min(a.x + a.width, b.x + b.width);
        const int y2 = std::min(a.y + a.height, b.y + b.height);
        const int iw = std::max(0, x2 - x1);
        const int ih = std::max(0, y2 - y1);
        const double intersection = static_cast<double>(iw) * ih;
        const double uni = static_cast<double>(a.area()) + static_cast<double>(b.area()) - intersection;
        return uni > 0.0 ? intersection / uni : 0.0;
    }

    static double centerDistance(const cv::Rect &a, const cv::Rect &b)
    {
        return cv::norm(centerOf(a) - centerOf(b));
    }

    static cv::Point2f centerOf(const cv::Rect &box)
    {
        return cv::Point2f(box.x + box.width * 0.5f, box.y + box.height * 0.5f);
    }

private:
    void applyMatch(PersonTrack &track, const PersonDetection &detection,
                    double now_s, double diagonal)
    {
        const cv::Point2f new_center = centerOf(detection.box);
        const double dt = now_s - track.last_seen;

        if (track.last_seen > 0.0 && dt > 0.01 && dt < 1.0)
        {
            cv::Point2f velocity = (new_center - track.center) / static_cast<float>(dt);
            const double speed = cv::norm(velocity);
            const double max_speed = params_.max_predict_speed_ratio * diagonal;
            if (speed > max_speed && speed > 1e-6)
                velocity *= static_cast<float>(max_speed / speed);

            if (track.has_velocity)
                track.velocity = 0.5f * track.velocity + 0.5f * velocity;
            else
            {
                track.velocity = velocity;
                track.has_velocity = true;
            }
        }

        track.box = detection.box;
        track.confidence = detection.confidence;
        track.last_seen = now_s;
        track.matched_this_frame = true;
        track.center = new_center;
    }

    Params params_;
    std::vector<PersonTrack> tracks_;
    int next_id_ = 1;
    int primary_id_ = -1;
};

#endif // SAFINA_PERSON_TRACKER_H
