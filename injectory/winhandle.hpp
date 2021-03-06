#pragma once
#include "injectory/exception.hpp"
#include "injectory/handle.hpp"


class WinHandle : public Handle<void>
{
public:
	WinHandle(handle_t handle = nullptr)
		: Handle<void>(handle)
	{}

	template <class Deleter>
	WinHandle(handle_t handle, Deleter deleter)
		: Handle<void>(handle, deleter)
	{}

public:
	DWORD wait(DWORD millis = INFINITE) const
	{
		DWORD ret = WaitForSingleObject(handle(), millis);
		if (ret == WAIT_FAILED)
		{
			DWORD errcode = GetLastError();
			BOOST_THROW_EXCEPTION(ex_wait_for_single_object() << e_api_function("WaitForSingleObject") << e_last_error(errcode) << e_handle(handle()));
		}
		else
			return ret;
	}

	static DWORD wait(const vector<handle_t>& handles, bool waitAll, DWORD millis = INFINITE)
	{
		DWORD ret = WaitForMultipleObjects(handles.size(), &handles[0], waitAll, millis);
		if (ret == WAIT_FAILED)
		{
			DWORD errcode = GetLastError();
			BOOST_THROW_EXCEPTION(ex_wait_for_multiple_objects() << e_api_function("WaitForMultipleObjects") << e_last_error(errcode) << e_handles(handles));
		}
		else
			return ret;
	}

public:
	static const WinHandle& std_in();
	static const WinHandle& std_out();
	static const WinHandle& std_err();
};
