//
// Created by Adrien COURNAND on 12/11/2022.
//

#ifndef E5150_SPDLOG_IMGUI_SINK_HPP
#define E5150_SPDLOG_IMGUI_SINK_HPP

#include "imgui.h"

#include "spdlog/sinks/base_sink.h"

template<typename Mutex>
class SpdlogImGuiColorSink : public spdlog::sinks::base_sink<Mutex>
{
public:
	void imguiDraw(void)
	{
		ImGui::Begin("Logging");
		ImGui::TextUnformatted(mLogTextBuffer.begin());
		ImGui::End();
	}

	void init()
	{
		mLogTextBuffer.reserve(1024*1024);
		mLogTextBuffer.append("");
	}

protected:
	void sink_it_(const spdlog::details::log_msg& msg) override
	{
		spdlog::memory_buf_t formatted;
		spdlog::sinks::base_sink<Mutex>::formatter_.get()->format(msg,formatted);
		mLogTextBuffer.append(fmt::to_string(formatted).c_str());
	}

	void flush_() override
	{
	}

private:
	ImGuiTextBuffer mLogTextBuffer;
};


#endif //E5150_SPDLOG_IMGUI_SINK_HPP
