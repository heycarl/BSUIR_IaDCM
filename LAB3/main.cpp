#include <iomanip>
#include <iostream>
#include <windows.h>
#include "hexioctrl.h"

using namespace std;

// Функции для чтения
// из портов ввода/вывода.
BYTE InPort(WORD port);
WORD InPortW(WORD port);

// Функция для записи в
// порты ввода/вывода.
void OutPort(WORD port, BYTE value);

// Макросы, представляющие команды для устройств ATA.
#define IDENTIFY_PACKET_DEVICE 0xA1 // для запроса инфы о дисководах
#define IDENTIFY_DEVICE 0xEC // для запроса инфы о жестких дисках

// Регистры устройств ATA.
const WORD AS[2] = {0x3F6, 0x376}; // альтернативный регистр состояния
const WORD DR[2] = {0x1F0, 0x170}; // регистр данных
const WORD DH[2] = {0x1F6, 0x176}; // регистр адресcа
const WORD SR[2] = {0x1F7, 0x177}; // регистр состояния
const WORD CR[2] = {0x1F7, 0x177}; // регистр команд

// Данные, получаемые из регистра данных.
WORD data[256]; // массив информации.

// Ожидает, пока устройство ATA занято.
void WaitBusy(int channel);

// Ожидает, пока устройство ATA не готово к работе.
BOOL WaitReady(int channel);

// Получение дискового устройства ATA, при этом записывая всю необходимую информацию в `data'.
BOOL GetATADevice(int channel, int device);

// Печать полученной информации, находящейся в `data'.
void PrintData(int channel, int device);

// Наличие HDD
bool HasHDD();

int main()
{
	ALLOW_IO_OPERATIONS;

	setlocale(LC_ALL, ".1251");

	// Печать разделительной линии.
	cout << setw(77) << setfill('=') << "" << endl;

	// Перебираем все возможные устройства ATA.
	int channel = 0;
	for (int device = 0; device < 2; ++device)
		if (GetATADevice(channel, device)) {
			PrintData(channel, device);
			cout << endl;
			cout << setw(77) << setfill('=') << "" << endl;
		}
	system("pause");
	return 0;
}

// Ожидает, пока устройство ATA занято.
void WaitBusy(int channel)
{
	BYTE state;

	// Ожидаем обнуления бита BSY (занятости).
	do
		state = InPort(AS[channel]); // Считываем состояние устройства ATA.
	while (state & 0x80); // Проверяем, установлен ли бит BSY (самый старший бит).

	// Когда бит BSY обнулится, устройство ATA будет готово к выполнению команд.
}

// Ожидает, пока устройство ATA не готово к работе.
BOOL WaitReady(int channel)
{
	for (int i = 0; i < 1000; ++i)
		// Если устройство ATA готово к работе...
		if (InPort(AS[channel]) & 0x40) // Проверяем бит DRDY (готовности).
			return true;

	// Если устройство не готово к работе после 1000 попыток, возвращаем false.
	return false;
}

// Проверяет наличие HDD.
bool HasHDD()
{
	int channel = 0; // Выбираем канал 0 (Primary Channel).
	int device = 0;  // Выбираем устройство 0 (Master).

	return GetATADevice(channel, device);
}

// Получение дискового устройства ATA, при этом записывая всю необходимую информацию в `data'.
BOOL GetATADevice(int channel, int device)
{
	// Массив, представляющий команды для получения информации об устройстве ATA.
	const BYTE commands[] = {
//			IDENTIFY_PACKET_DEVICE, // Добавить информацию о сидиромах.
			IDENTIFY_DEVICE,        // Добавить информацию о всех дисках.
	};

	// Перебор всех команд.
	for (int i = 0; i < sizeof(commands); ++i) {
		// Ожидаем обнуления бита BSY.
		WaitBusy(channel);

		// Формируем адрес устройства ATA и устанавливаем бит 7 (LBA, Logical Block Addressing).
		OutPort(DH[channel], (device << 4) | 0xE0);

		// Дожидаемся признака готовности, если устройство ATA присутствует.
		if (!WaitReady(channel))
			return false;

		// Записываем код команды в регистр команд.
		OutPort(CR[channel], commands[i]);

		// Ожидаем обнуления бита BSY.
		WaitBusy(channel);

		// Проверка на ошибку.
		if (!(InPort(SR[channel]) & 0x08)) {
			// Если ошибка произошла при обработке команды IDENTIFY_DEVICE...
			if (i == 1)
				return false;

			continue;
		} else
			break;
	}

	// Получение конфигурационного блока информации об устройстве ATA.
	for (int i = 0; i < 256; ++i)
		data[i] = InPortW(DR[channel]);

	return true;
}

// Печать полученной информации об устройстве ATA.
void PrintData(int channel, int device)
{

	cout << setw(30) << setfill(' ') << left << "Расположение:";
	cout << channel << ", " << device;

	// Печать типа устройства ATA.
	cout << endl << endl << setw(29) << setfill(' ') << left << "Тип:";
	for (int i = 29; i <= 46; ++i)
		cout << (CHAR)(data[i] >> 8) << (CHAR)(data[i] & 0x00FF);

	// Печать модели устройства ATA.
	cout << endl << endl << setw(30) << setfill(' ') << left << "Модель:";
	for (int i = 27; i <= 46; ++i)
		cout << (CHAR)(data[i] >> 8) << (CHAR)(data[i] & 0x00FF);

	// Печать серийного номера устройства ATA.
	cout << endl << setw(30) << setfill(' ') << left << "Серийный номер:";
	for (int i = 10; i <= 19; ++i)
		cout << (CHAR)(data[i] >> 8) << (CHAR)(data[i] & 0x00FF);

	// Печать версии прошивки устройства ATA.
	cout << endl << setw(30) << setfill(' ') << left << "Версия прошивки:";
	for (int i = 23; i <= 26; ++i)
		cout << (CHAR)(data[i] >> 8) << (CHAR)(data[i] & 0x00FF);

	// Печать общего объема устройства в МБ.
	cout << endl << endl << setw(30) << setfill(' ') << left << "Общий объем:";
	cout << setprecision(0) << (long double)(((ULONG *)data)[0]) * 512 / 1024 / 1024 << " МБ";

	// Печать размера диска в МБ.
	cout << endl << setw(30) << setfill(' ') << left << "Размер диска:";
	cout << setprecision(0) << (long double)(((ULONG *)data)[30]) * 512 / 1024 / 1024 << " МБ";

	// Печать размера блока памяти на диске в МБ.
	cout << endl << setw(30) << setfill(' ') << left << "Размер блока памяти:";
	cout << setprecision(0) << (long double)(((ULONG *)data)[1]) * 512 / 1024 / 1024 << " МБ" << endl;

	if (HasHDD()) {
		cout << "HDD обнаружен." << endl;
	} else {
		cout << "HDD не обнаружен." << endl;
	}

	// Печать интерфейса устройства ATA.
	cout << endl << setw(30) << setfill(' ') << left << "Интерфейс:";
	// Определение интерфейса на основе данных в регистре IDENTIFY DEVICE.
	WORD interf = (data[168] & 0x000F); // Индекс 168 в данных ATA.

	switch (interf) {
	case 0x0001:
		cout << "Parallel ATA (PATA)";
		break;
	case 0x0002:
		cout << "Serial ATA (SATA)";
		break;
	case 0x0003:
		cout << "Serial Attached SCSI (SAS)";
		break;
	default:
		cout << "Неизвестный интерфейс";
	}

	// Печать поддержки режимов PIO.
	cout << endl << endl << "Поддержка PIO:";
	cout << endl << "  (" << (data[64] & 0x1 ? "+" : "-") << ") PIO 3";
	cout << endl << "  (" << (data[64] & 0x2 ? "+" : "-") << ") PIO 4";

	// Печать поддержки режимов Multiword DMA.
	cout << endl << endl << "Поддержка Multiword DMA:";
	cout << endl << "  (" << (data[63] & 0x1 ? "+" : "-") << ") MWDMA 0";
	cout << endl << "  (" << (data[63] & 0x2 ? "+" : "-") << ") MWDMA 1";
	cout << endl << "  (" << (data[63] & 0x4 ? "+" : "-") << ") MWDMA 2";

	// Печать поддержки режимов Ultra DMA.
	cout << endl << endl << "Поддержка Ultra DMA:";
	cout << endl << "  (" << (data[88] & 0x1 ? "+" : "-") << ") UDMA 0";
	cout << endl << "  (" << (data[88] & 0x2 ? "+" : "-") << ") UDMA 1";
	cout << endl << "  (" << (data[88] & 0x4 ? "+" : "-") << ") UDMA 2";
	cout << endl << "  (" << (data[88] & 0x8 ? "+" : "-") << ") UDMA 3";
	cout << endl << "  (" << (data[88] & 0x10 ? "+" : "-") << ") UDMA 4";
	cout << endl << "  (" << (data[88] & 0x20 ? "+" : "-") << ") UDMA 5";

	// Печать поддерживаемых версий ATA.
	cout << endl << endl << "Поддержка версий ATA:";
	cout << endl << "  (" << (data[80] & 0x2 ? "+" : "-") << ") ATA 1";
	cout << endl << "  (" << (data[80] & 0x4 ? "+" : "-") << ") ATA 2";
	cout << endl << "  (" << (data[80] & 0x8 ? "+" : "-") << ") ATA 3";
	cout << endl << "  (" << (data[80] & 0x10 ? "+" : "-") << ") ATA 4";
	cout << endl << "  (" << (data[80] & 0x20 ? "+" : "-") << ") ATA 5";
	cout << endl << "  (" << (data[80] & 0x40 ? "+" : "-") << ") ATA 6";
	cout << endl << "  (" << (data[80] & 0x80 ? "+" : "-") << ") ATA 7";

	cout << endl;
	cout << endl; cout << endl; cout << endl; cout << endl; cout << endl;


}

// Функция для считывания 8-битного значения из порта.
BYTE InPort(WORD port)
{
	BYTE result;

	__asm {
			mov DX, port    // Передаем номер порта в DX.
			in AL, DX       // Считываем байт из порта в AL.
			mov result, AL  // Записываем считанный байт в переменную result.
	}

	return result;
}

// Функция для считывания 16-битного значения из порта.
WORD InPortW(WORD port)
{
	WORD result;

	__asm {
			mov DX, port    // Передаем номер порта в DX.
			in AX, DX       // Считываем слово (16 бит) из порта в AX.
			mov result, AX  // Записываем считанное слово в переменную result.
	}

	return result;
}

// Функция для записи 8-битного значения в порт.
void OutPort(WORD port, BYTE value)
{
	__asm {
			mov DX, port    // Передаем номер порта в DX.
			mov AL, value   // Загружаем 8-битное значение в AL.
			out DX, AL      // Записываем AL в порт DX.
	}
}

// Функция для записи 16-битного значения в порт.
void OutPort(WORD port, WORD value)
{
	__asm {
			mov DX, port    // Передаем номер порта в DX.
			mov AX, value   // Загружаем 16-битное значение в AX.
			out DX, AX      // Записываем AX в порт DX.
	}
}
