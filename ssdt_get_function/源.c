#include <ntddk.h>
#include <windef.h>
#include <intrin.h>
#pragma intrinsic(__readmsr)
typedef struct _KeServiceDescriptorTable
{
	PVOID ServiceTableBase; // ָ��_KiServiceTable���ַ
	PULONG ServiceCounterTableBase; // ָ����ô������ַ
	ULONG NumberOfService; // ��������
	PULONG ParamTableBase; //ָ��������ַ

}KeServiceDescriptorTable, * PKeServiceDescriptorTable;

ULONGLONG Get_SSTD_Base()
{
	PUCHAR Base = (PUCHAR)__readmsr(0xC0000082);      // ��ȡC0000082�Ĵ���
	PUCHAR Address = Base + 0x500;  // �������޵�ַ

	PUCHAR i = NULL;
	UCHAR b1 = 0, b2 = 0, b3 = 0;                     // ����������
	ULONG templong = 0;
	ULONGLONG addr = 0;                               // ����ȡ���ĵ�ַ
	for (i = Base; i < Address; i++)
	{
		if (MmIsAddressValid(i) && MmIsAddressValid(i + 1) && MmIsAddressValid(i + 2))
		{
			b1 = *i; b2 = *(i + 1); b3 = *(i + 2);
			if (b1 == 0x4c && b2 == 0x8d && b3 == 0x15)   // �ж��Ƿ�=4c8d15
			{
				memcpy(&templong, i + 3, 4);              // ��i+3λ�ÿ���������4�ֽ�

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
		PULONG tablebase = keservicedescriptortable->ServiceTableBase;//��ȡtablebase
		ULONGLONG temp = tablebase[index];//������Ż�ȡƫ��
		temp = temp >> 4;//���м���õ���ʵƫ��
		ULONGLONG funaddr = (ULONGLONG)tablebase + temp;//tablesbase+ƫ�Ƶõ�������ַ
		return funaddr;
	}
	else
	{
		DbgPrint("��ȡʧ��");
		return 0;
	}
}

void Unload(PDRIVER_OBJECT driverobj)
{
	DbgPrint("ж�سɹ�");
}
NTSTATUS DriverEntry(PDRIVER_OBJECT driverobj, PUNICODE_STRING seg_path)
{
	DbgPrint("������ʼ����");
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