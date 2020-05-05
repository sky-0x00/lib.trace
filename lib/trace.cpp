#include <cassert>
#include <map>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>					// Windows Header Files

#define LIBRARY_TRACE__EXPORT
#include "trace.h"

//-------------------------------------------------------------------------------------------------------------------------------------------
class time {
public:
	typedef SYSTEMTIME value;
public:
	static void get(_out value &value) noexcept;
	static value get() noexcept;
public:
	time() noexcept;
	const value& get(_in bool is_forse) noexcept;
private:
	value _value;
};

/*static*/ void time::get(
	_out value &value
) noexcept {
	Winapi::GetLocalTime(&value);
}
/*static*/ time::value time::get(
) noexcept {
	value value;
	get(value);
	return value;
}

time::time(
) noexcept {
	get(_value);
}

const time::value& time::get(
	_in bool is_forse
) noexcept {
	if (is_forse)
		get(_value);
	return _value;
}

//-------------------------------------------------------------------------------------------------------------------------------------------
using namespace trace;

//-------------------------------------------------------------------------------------------------------------------------------------------
/*static*/ string_t tracer::settings::get_module(
	_in bool without_extension /*= false*/
) {
	// QueryFullProcessImageNameW
	// GetProcessImageFileNameW
	// GetModuleFileNameW/GetModuleFileNameExW
	
	std::pair<std::unique_ptr<char_t>, unsigned> buffer;
	for (buffer.second = 10; ; buffer.second += 64) {

		buffer.first.reset(new char_t[buffer.second]);
		const auto size = Winapi::GetModuleFileNameW(NULL, buffer.first.get(), buffer.second);
		if (stdex::is__in_range(size, {1, buffer.second})) {
			buffer.second = size;
			break;
		}
		const auto LastError = Winapi::GetLastError();
		if (ERROR_INSUFFICIENT_BUFFER != LastError)
			throw LastError;
	}

	std::pair<cstr_t, cstr_t> path = std::make_pair(buffer.first.get(), buffer.first.get() + buffer.second);
	{
		decltype(path) found = { nullptr, nullptr };
		for (cstr_t p = path.second; --p != path.first; ) {
			switch (*p) {
			case L'.':
				if (!found.second)
					found.second = p;
				break;
			case L'\\':
				if (!found.first) {
					found.first = p;
					goto go_next;
				}
				break;
			}
		}

	go_next:
		if (found.first)
			path.first = found.first + 1;
		if (without_extension && found.second)
			path.second = found.second;
	}

	return { path.first, path.second };
}

tracer::settings::settings(
	_in const config &config
) noexcept :
	output(config.output), show_flag(config.show_flag)
{
	module = stdex::is_null(config.module) ? get_module() : config.module;
	if (!stdex::is_null(config.file.path) && output.check(output::file) ) {

		std::ios_base::openmode openmode = std::ios_base::out;			// auto doesn't work
		if (config.file.openmode__is_append)
			openmode |= std::ios_base::app | std::ios_base::ate;
		file.open(config.file.path, openmode);
		
		if (file.is_open())
			if (config.file.openmode__is_append && (0 < file.tellp()))
				file.put(L'\n');
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------
tracer::string::string(
	_in unsigned size /*= 4096*/
) :
	_size(0)
{
	_buffer.data.reset(new char_t[size]);
	assert(_buffer.data.get());
	_buffer.size = size;
}

//unsigned tracer::string::format(
//	_in unsigned offset, _in cstr_t format, _in ...
//) {
//	va_list args;
//	va_start(args, format);
//	const auto result = string::format(offset, format, args);
//	va_end(args);
//	return result;
//}
unsigned tracer::string::format(
	_in unsigned offset, _in cstr_t format, _in va_list args
) {
	assert(_size < _buffer.size);
	auto result = vswprintf_s(offset + _buffer.data.get(), _buffer.size - offset, format, args);
	if (0 > result) {
		// ...
		result = 0;
	}
	return result;
}

unsigned tracer::string::format(
	_in cstr_t format, _in va_list args
) {
	_size = string::format(0, format, args);
	return _size;
}
unsigned tracer::string::format_append(
	_in cstr_t format, _in va_list args
) {
	_size += string::format(_size, format, args);
	return _size;
}

unsigned tracer::string::format(
	_in cstr_t format, _in ...
) {
	va_list args;
	va_start(args, format);
	const auto result = string::format(format, args);
	va_end(args);
	return result;
}
unsigned tracer::string::format_append(
	_in cstr_t format, _in ...
) {
	va_list args;
	va_start(args, format);
	const auto result = string::format_append(format, args);
	va_end(args);
	return result;
}

unsigned tracer::string::append(
	_in char_at ch
) {
	assert(_size < _buffer.size);
	if (_size + 1 != _buffer.size) {
		_buffer.data.get()[_size++] = ch;
		_buffer.data.get()[_size] = L'\0';
	}
	return _size;
}

unsigned tracer::string::size(
) const {
	return _size;
}
cstr_t tracer::string::c_str() const {
	return _buffer.data.get();
}

//-------------------------------------------------------------------------------------------------------------------------------------------
tracer::tracer(
	_in const config &config
) :
	_settings(config)
{
	const auto time {time::get()};
	//const auto pid = Winapi::GetCurrentProcessId();													// without pid(dec)

	// == <<|2020-01-09 0:15:45.942| ===========================================================================================
	//_string.format(L"%s %s|%04hu-%02hu-%02hu %hu:%02hu:%02hu.%03hu|0x%X(%u)| %s\n", 
	_string.format(L"%s %s|%04hu-%02hu-%02hu %hu:%02hu:%02hu.%03hu|0x%X| %s\n",							// without pid(dec)
		string_t(_padding.line_separator.count.first, _padding.line_separator.ch).c_str(),
		string_t(_padding.ctor_dtor.count, _padding.ctor_dtor.ch.first).c_str(),
		time.wYear, time.wMonth, time.wDay,
		time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, 
		Winapi::GetCurrentProcessId(),	//pid, pid,														// without pid(dec)
		string_t(_padding.line_separator.count.second, _padding.line_separator.ch).c_str()
	);
	output();
}
tracer::~tracer(
) {
	const auto time {time::get()};
	//const auto pid = Winapi::GetCurrentProcessId();													// without pid(dec)

	// == >>|2020-01-09 0:15:45.942| ===========================================================================================
	//_string.format(L"%s %s|%04hu-%02hu-%02hu %hu:%02hu:%02hu.%03hu|0x%X(%u)| %s\n",
	_string.format(L"%s %s|%04hu-%02hu-%02hu %hu:%02hu:%02hu.%03hu|0x%X| %s\n",							// without pid(dec)
		string_t(_padding.line_separator.count.first, _padding.line_separator.ch).c_str(),
		string_t(_padding.ctor_dtor.count, _padding.ctor_dtor.ch.second).c_str(),
		time.wYear, time.wMonth, time.wDay,
		time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
		Winapi::GetCurrentProcessId(),	//pid, pid,														// without pid(dec)
		string_t(_padding.line_separator.count.second, _padding.line_separator.ch).c_str()
	);
	output();
}

/*protected*/ unsigned tracer::trace(
	_in categoty category, 
	_in cstr_t function /*= __FUNCTIONW__*/, 
	_in cstr_t format, 
	_in va_list args
) {
	string__create_prefix(function);
	string__append_userdata(category, format, args);

	output();

	return _string.size() /*- 1*/;
}

/*protected*/ unsigned tracer::trace_n(
	_in cstr_t function /*= __FUNCTIONW__*/,
	_in cstr_t format,
	_in va_list args
) {
	return trace(categoty::normal, function, format, args);
}
/*protected*/ unsigned tracer::trace_w(
	_in cstr_t function /*= __FUNCTIONW__*/,
	_in cstr_t format,
	_in va_list args
) {
	return trace(categoty::warning, function, format, args);
}
/*protected*/ unsigned tracer::trace_e(
	_in cstr_t function /*= __FUNCTIONW__*/,
	_in cstr_t format,
	_in va_list args
) {
	return trace(categoty::error, function, format, args);
}

unsigned tracer::trace(
	_in categoty category, 
	_in cstr_t function /*= __FUNCTIONW__*/,
	_in cstr_t format, 
	_in ...
) {
	va_list args;
	va_start(args, format);
	const auto result = trace(category, function, format, args);
	va_end(args);
	return result;
}

unsigned tracer::trace_n(
	_in cstr_t function /*= __FUNCTIONW__*/, 
	_in cstr_t format, 
	_in ...
) {
	va_list args;
	va_start(args, format);
	const auto result = trace_n(function, format, args);
	va_end(args);
	return result;
}
unsigned tracer::trace_w(
	_in cstr_t function /*= __FUNCTIONW__*/,
	_in cstr_t format,
	_in ...
) {
	va_list args;
	va_start(args, format);
	const auto result = trace_w(function, format, args);
	va_end(args);
	return result;
}
unsigned tracer::trace_e(
	_in cstr_t function /*= __FUNCTIONW__*/, 
	_in cstr_t format, 
	_in ...
) {
	va_list args;
	va_start(args, format);
	const auto result = trace_e(function, format, args);
	va_end(args);
	return result;
}

//void tracer::string__append_n(
//) {
//	_string.append(L'\n');
//}

void tracer::string__create_prefix(
	_in cstr_t function /*= __FUNCTIONW__*/
) {
	const auto time {time::get()};
	_string.format(L"%hu:%02hu:%02hu.%03hu|%s|", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds, _settings.module.c_str());
	if (_settings.show_flag.check(trace::show_flag::process__trace_id)) {
		//const auto pid = Winapi::GetCurrentProcessId();
		//_string.format_append(L"0x%X(%u):", pid, pid);
		_string.format_append(L"0x%X:", Winapi::GetCurrentProcessId());
	} {
		//const auto tid = Winapi::GetCurrentThreadId();
		//_string.format_append(L"0x%X(%u)|", tid, tid);
		_string.format_append(L"0x%X|", Winapi::GetCurrentThreadId());
	}
	//_string.format_append(L"%s()|", function);
	_string.format_append(L"%s|", function);
}
void tracer::string__append_userdata(
	_in categoty category, _in cstr_t format, _in va_list args
) {
	static const std::map<categoty, char_t> map_tc{
		{categoty::normal,  L'N'},
		{categoty::warning, L'W'},
		{categoty::error,   L'E'},
	};
	_string.format_append(L"%c|", map_tc.at(category));

	string_t format_new(format);
	format_new.push_back(L'\n');
	_string.format_append(format_new.c_str(), args);
}

void tracer::output(
) const {
	if (_settings.output.check(trace::output::debug_monitor))
		output_to__monitor();
	if (_settings.output.check(trace::output::file))
		output_to__file();
}

void tracer::output_to__monitor(
) const {
	Winapi::OutputDebugStringW(_string.c_str());
}
void tracer::output_to__file(
) const {
	if (_settings.file.is_open())
		_settings.file.write(_string.c_str(), _string.size());
}

//-------------------------------------------------------------------------------------------------------------------------------------------
