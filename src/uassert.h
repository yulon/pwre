#ifndef _UASSERT_H
#define _UASSERT_H

#ifdef __cplusplus
	#ifdef _WIN32
		#include <windows.h>
		#include <sstream>
		#define uassert(_exp, _part, _culprit) { \
			if (!(_exp)) { \
				std::ostringstream ss; \
				ss << "File: [" << _part << "] " << __FILE__ << ":" << __LINE__ << "\r\n\r\nExpression: " << #_exp << "\r\n\r\nCulprit: " << _culprit; \
				MessageBoxA(0, ss.str().c_str(), "Assertion failed!", MB_OK | MB_ICONERROR); \
				exit(EXIT_FAILURE); \
			} \
		}
	#else
		#include <iostream>
		#define uassert(_exp, _part, _culprit) { \
			if (!(_exp)) { \
				std::cout << "Assertion failed!" << std::endl << "File: [" << _part << "] " << __FILE__ << ":" << __LINE__ << std::endl << "Expression: " << #_exp << std::endl << "Culprit: " << _culprit << std::endl; \
				exit(EXIT_FAILURE); \
			} \
		}
	#endif
#else
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#ifdef _WIN32
		#include <windows.h>
		#define uassert(_exp, _part, _culprit) { \
			if (!(_exp)) { \
				const char *fmt = "File: [%s] %s:%u\r\n\r\nExpression: %s\r\n\r\nCulprit: %s"; \
				int buf_len = strlen(fmt) + strlen(_part) + strlen(__FILE__) + 10 + strlen(#_exp) + strlen(_culprit); \
				char *buf = (char *)calloc(1, buf_len); \
				sprintf_s(buf, buf_len, "File: [%s] %s:%u\r\n\r\nExpression: %s\r\n\r\nCulprit: %s", _part, __FILE__, __LINE__, #_exp, _culprit); \
				MessageBoxA(0, buf, "Assertion failed!", MB_OK | MB_ICONERROR); \
				exit(EXIT_FAILURE); \
			} \
		}
	#else
		#define uassert(_exp, _part, _culprit) { \
			if (!(_exp)) { \
				printf_s("Assertion failed!\nFile: [%s] %s:%u\nExpression: %s\nCulprit: %s", _part, __FILE__, __LINE__, #_exp, _culprit); \
				exit(EXIT_FAILURE); \
			} \
		}
	#endif
#endif

#endif // !_UASSERT_H
