#pragma once

#include <fstream>
#include <initializer_list>
#include "..\..\..\..\units\macro-defs.h"
#include "..\..\..\..\units\system-types.h"
#include "..\..\..\..\units\system-routines.h"
#include "..\..\..\..\units\string.h"

#ifdef LIBRARY_TRACE__EXPORT
	#define LIBRARY_TRACE __declspec(dllexport)
#else 
	#define LIBRARY_TRACE __declspec(dllimport)
#endif

#define TRACE_INIT(config)					trace::tracer __tracer {config}
#define TRACE(category, format, ...)		__tracer.trace(__FUNCTIONW__, category, format, __VA_ARGS__)
#define TRACE_N(format, ...)				__tracer.trace_n(__FUNCTIONW__, format, __VA_ARGS__)
#define TRACE_W(format, ...)				__tracer.trace_w(__FUNCTIONW__, format, __VA_ARGS__)
#define TRACE_E(format, ...)				__tracer.trace_e(__FUNCTIONW__, format, __VA_ARGS__)

namespace trace {

	enum class categoty {
		normal /*information*/ = 0, 
		warning, 
		error
	};

	// исползуется в битовой маске 
	enum class output {
		debug_monitor = 1,											// debug-monitor, см. Winapi::OutputDebugStringW()
		file = 2 /*, console*/										// файл на диске
	};

	// исползуется в битовой маске 
	enum class show_flag {
		process__breaf_info = 1,									// выводить breaf-info о текущем процессе в c_tor()'е, не реализовано пока
		process__trace_id = 2										// выводить pid в каждом trace-output'е (перед выводом tid)
	};


	class LIBRARY_TRACE tracer {

	public:
		struct config {

			_optional cstr_t module = nullptr;							// если не указано (nullptr), будет использоваться имя файла образа текущего процесса
			struct file {
				_optional cstr_t path = nullptr;						// если не указано (nullptr), трассировка в файл выводиться не будет
				bool openmode__is_append = false;						// если true, то файл будет открыт в режиме добавления новых трасс, иначе - содержимое будет перезатерто при открытии
			};
			struct file file;
		
			std::initializer_list<output> output /*= {output::debug_monitor}*/;
			std::initializer_list<show_flag> show_flag /*= {show_flag::process__breaf_info}*/;
		};

	protected:
		class settings {

		public:
			settings(_in const config &config) noexcept;
		private:
			static string_t get_module(_in bool without_extension = false);

		public:
			string_t module;
			mutable std::wofstream file;
			stdex::mask<output> output;
			stdex::mask<show_flag> show_flag;
		};

		class string {

		protected:
			struct buffer {
				std::unique_ptr<char_t> data;
				unsigned size;
			};
			struct buffer _buffer;
			unsigned _size;

		public:
			string(_in unsigned size = 4096);

			unsigned size() const;
			cstr_t c_str() const;

		public:
			unsigned format(_in cstr_t format, _in va_list args);									// возвращает size
			unsigned format_append(_in cstr_t format, _in va_list args);							// возвращает size
			unsigned format(_in cstr_t format, _in ...);											// возвращает size
			unsigned format_append(_in cstr_t format, _in ...);										// возвращает size

			unsigned append(_in char_at ch);														// возвращает size

		protected:
			//unsigned format(_in unsigned offset, _in cstr_t format, _in ...);
			unsigned format(_in unsigned offset, _in cstr_t format, _in va_list args);				// не изменяет size, возвращает число записанных символов
		};

		struct padding {
			struct line_separator {
				char_t ch;
				std::pair<unsigned, unsigned> count;
			};
			struct ctor_dtor {
				std::pair<char_t, char_t> ch;
				unsigned count;
			};
			struct line_separator line_separator;
			struct ctor_dtor ctor_dtor;
		};
		
	public:
		tracer(_in const config &config);
		~tracer();

		unsigned trace(_in categoty category, _in cstr_t function /*= __FUNCTIONW__*/, _in cstr_t format, _in ...);
		unsigned trace_n(_in cstr_t function /*= __FUNCTIONW__*/, _in cstr_t format, _in ...);
		unsigned trace_w(_in cstr_t function /*= __FUNCTIONW__*/, _in cstr_t format, _in ...);
		unsigned trace_e(_in cstr_t function /*= __FUNCTIONW__*/, _in cstr_t format, _in ...);

	protected:
		unsigned trace(_in categoty category, _in cstr_t function /*= __FUNCTIONW__*/, _in cstr_t format, _in va_list args);		
		unsigned trace_n(_in cstr_t function /*= __FUNCTIONW__*/, _in cstr_t format, _in va_list args);
		unsigned trace_w(_in cstr_t function /*= __FUNCTIONW__*/, _in cstr_t format, _in va_list args);
		unsigned trace_e(_in cstr_t function /*= __FUNCTIONW__*/, _in cstr_t format, _in va_list args);

		void string__create_prefix(_in cstr_t function /*= __FUNCTIONW__*/);
		void string__append_userdata(_in categoty category, _in cstr_t format, _in va_list args);
		//void string__append_n();

		void output() const;
		void output_to__monitor() const;
		void output_to__file() const;

	protected:
		static constexpr padding _padding {{L'=', {96, 4}}, {{L'<', L'>'}, 2}};

	private:	
		string _string;
		const settings _settings;
	};

}	// namespace trace