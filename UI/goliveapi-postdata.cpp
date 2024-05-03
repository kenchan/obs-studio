#include "goliveapi-postdata.hpp"

#include <nlohmann/json.hpp>

#include "system-info.hpp"

nlohmann::json
constructGoLivePost(QString streamKey,
		    const std::optional<uint64_t> &maximum_aggregate_bitrate,
		    const std::optional<uint32_t> &maximum_video_tracks,
		    bool vod_track_enabled,
		    const std::map<std::string, video_t *> &extra_views)
{
	using json = nlohmann::json;

	json post_data;
	post_data["service"] = "IVS";
	post_data["schema_version"] = "2023-05-10";
	post_data["authentication"] = streamKey.toStdString();

	auto &capabilities = post_data["capabilities"];
	capabilities["gpu"] = system_gpu_data();
	capabilities.update(system_info(), true);

	auto &client = capabilities["client"];

	client["vod_track_audio"] = vod_track_enabled;

	obs_video_info ovi;
	if (obs_get_video_info(&ovi)) {
		client["name"] = "obs-studio";
		client["version"] = obs_get_version_string();
		client["width"] = ovi.output_width;
		client["height"] = ovi.output_height;
		client["fps_numerator"] = ovi.fps_num;
		client["fps_denominator"] = ovi.fps_den;

		client["canvas_width"] = ovi.base_width;
		client["canvas_height"] = ovi.base_height;
	}

	auto &json_extra_views = capabilities["extra_views"];
	for (auto &video_output : extra_views) {
		video_t *video = video_output.second;
		if (!video)
			continue;
		const struct video_output_info *voi =
			video_output_get_info(video);
		if (!voi)
			continue;

		json view;
		view["name"] = video_output.first.c_str();
		view["canvas_width"] = voi->width;
		view["canvas_height"] = voi->height;
		view["fps_numerator"] = voi->fps_num;
		view["fps_denominator"] = voi->fps_den;

		json_extra_views.push_back(view);
	}

	auto &preferences = post_data["preferences"];
	if (maximum_aggregate_bitrate.has_value())
		preferences["maximum_aggregate_bitrate"] =
			maximum_aggregate_bitrate.value();
	if (maximum_video_tracks.has_value())
		preferences["maximum_video_tracks"] =
			maximum_video_tracks.value();

	return post_data;
}
