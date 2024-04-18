#pragma once

#include <obs.hpp>
#include <optional>
#include <QString>
#include <nlohmann/json_fwd.hpp>

nlohmann::json
constructGoLivePost(QString streamKey,
		    const std::optional<uint64_t> &maximum_aggregate_bitrate,
		    const std::optional<uint32_t> &maximum_video_tracks,
		    bool vod_track_enabled);
