#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <iostream>

#include "component.hpp"

enum class LOG_LEVEL
{ INFO, ERROR, DEBUG };

static void LOG (const LOG_LEVEL logLevel, const std::string& msg, const std::string& prefix = "")
{
	std::ostream* stream = &std::cout;
	switch (logLevel)
	{
	case LOG_LEVEL::INFO:
		*stream << "INFO";
		break;
	
	case LOG_LEVEL::ERROR:
	{
		stream = &std::cerr;
		*stream << "ERROR";
	} break;
	
	#ifdef DEBUG
		case LOG_LEVEL::DEBUG:
			*stream << "DEBUG";
	#else
		default:
			//Nothinh is printed if this function called with debug log level when nont compiled in debug mode
			return;
		#endif
	}

	if (!prefix.empty())
		*stream << " (" << prefix << ")";
	*stream << ": " << msg << std::endl;
}

static void LOG(const std::string& msg)
{ LOG(LOG_LEVEL::INFO,msg); }

static void LOG(const E5150::Component& component, const std::string& msg)
{ LOG(LOG_LEVEL::INFO,msg,component.m_name); }

static void INFO (const E5150::Component& component, const std::string& msg)
{ LOG(LOG_LEVEL::INFO,msg,component.m_name); }

static void ERROR(const E5150::Component& component, const std::string& msg)
{ LOG(LOG_LEVEL::ERROR,msg,component.m_name); }

void DEBUG(const E5150::Component& component, const std::string& msg)
{
	#ifdef DEBUG
		LOG(LOG_LEVEL::DEBUG,msg,component.m_name);
	#endif
}

#endif