#include <ntddk.h>
#include <windef.h>
#include <intrin.h>
#pragma intrinsic(__readmsr)
typedef struct _KeServiceDescriptorTable
{
	PVOID ServiceTableBase; // 指向_KiServiceTable表地址
	PULONG ServiceCounterTableBase; // 指向调用次数表地址
	ULONG NumberOfService; // 函数个数
	PULONG ParamTableBase; //指向参数表地址

}KeServiceDescriptorTable, * PKeServiceDescriptorTable;

ULONGLONG Get_SSTD_Base()
{
	PUCHAR Base = (PUCHAR)__readmsr(0xC0000082);      // 读取C0000082寄存器
	PUCHAR Address = Base + 0x500;  // 设置上限地址

	PUCHAR i = NULL;
	UCHAR b1 = 0, b2 = 0, b3 = 0;                     // 保存特征码
	ULONG templong = 0;
	ULONGLONG addr = 0;                               // 最后获取到的地址
	for (i = Base; i < Address; i++)
	{
		if (MmIsAddressValid(i) && MmIsAddressValid(i + 1) && MmIsAddressValid(i + 2))
		{
			b1 = *i; b2 = *(i + 1); b3 = *(i + 2);
			if (b1 == 0x4c && b2 == 0x8d && b3 == 0x15)   // 判断是否=4c8d15
			{
				memcpy(&templong, i + 3, 4);              // 在i+3位置拷贝，拷贝4字节

				addr = (ULONGLONG)templong + (ULONGLONG)i + 7;
				return addr;
			}
		}
	}
	return 0;

}

ULONGLONG Get_Function_add(PKeServiceDescriptorTable keservicedescriptortable,DWORD index)
{
	if (keservicedescriptortable != NULL && index>0)
	{
		PULONG tablebase = keservicedescriptortable->ServiceTableBase;//获取tablebase
		ULONGLONG temp = tablebase[index];//按照序号获取偏移
		temp = temp >> 4;//进行计算得到真实偏移
		ULONGLONG funaddr = (ULONGLONG)tablebase + temp;//tablesbase+偏移得到函数地址
		return funaddr;
	}
	else
	{
		DbgPrint("获取失败");
		return 0;
	}
}

void Unload(PDRIVER_OBJECT driverobj)
{
	DbgPrint("卸载成功");
}
NTSTATUS DriverEntry(PDRIVER_OBJECT driverobj, PUNICODE_STRING seg_path)
{
	DbgPrint("驱动开始加载");
	const PKeServiceDescriptorTable servicetable = Get_SSTD_Base();
	if (servicetable == 0)
	{
		return STATUS_SUCCESS;
	}
	DWORD index = 20;
	ULONGLONG funaddr = Get_Function_add(servicetable, index);
	DbgPrint("function_addr:%p", funaddr);
	driverobj->DriverUnload = Unload;
	return STATUS_SUCCESS;
}