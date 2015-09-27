#pragma once
#include "injectory/common.hpp"
#include "injectory/exception.hpp"
#include "injectory/process.hpp"

class Module
{
	friend Module Process::isInjected(HMODULE);
	friend Module Process::isInjected(const Library&);
private:
	Process process;
	HMODULE handle_;

private:
	Module(HMODULE module, const Process& process)
		: handle_(module)
		, process(process)
	{}

public:
	Module()
		: Module(nullptr, Process())
	{}

	Module(const wstring& moduleName)
		: Module(GetModuleHandleW(moduleName.c_str()), Process::current)
	{
		if (!handle_)
			BOOST_THROW_EXCEPTION(ex_get_module_handle() << e_text("could not get handle to module '" + std::to_string(moduleName) + "'"));
	}

	Module(const string& moduleName)
		: Module(std::to_wstring(moduleName))
	{}

public:
	HMODULE handle() const
	{
		return handle_;
	}

public:
	FARPROC getProcAddress(string procName) const
	{
		FARPROC procAddress = GetProcAddress(handle_, procName.c_str());
		if (!procAddress)
			BOOST_THROW_EXCEPTION(ex_injection() << e_text("could not get the address of '"+procName+"'"));
		return procAddress;
	}

	template<class R, class... A>
	std::function<R(A...)> getProcAddress(string procName) const
	{
		return std::function<R(A...)>(reinterpret_cast<R (WINAPI *)(A...)>(getProcAddress(procName)));
	}

	wstring filename() const;
	wstring mappedFilename(bool throwOnFail = true) const;
	void eject();
public:
	static const Module& exe();
	static const Module kernel32;
	static const Module ntdll;
public:
	operator bool() const
	{
		return handle() != nullptr;
	}
};
