#include "scpi.h"
#include "system.h"
#include "outputControl.h"
#include <string.h>

SCPI_TypeDef scpi(&uart1);

void SCPI_UARTReceive_Handler(volatile char *data) {
	scpi.UARTReceive_Handler((char*)data);
}

void SCPI_TypeDef::UARTReceive_Handler(char *data) {
parseMnemonic:
	// SCPI Command Header
	if (strSkim(&data, "*")) {
		if (strMatch(data, "IDN?")) {
			Return("Convetech,CurveTracer,SCT-2001,v1.00a");
		}
	}
	// SCPI Instrument Control Header
	else if(strSkim(&data, ":")) {
		// SOURce1
		if (strSkim(&data, "SOURce1:") || strSkim(&data, "SOUR1:")) {
			// VOLTage
			if (strSkim(&data, "VOLTage") || strSkim(&data, "VOLT")) {
				// Query
				if (strMatch(data, "?")) {
					Return(outCtl.ch1.GetSetVoltage());
				}
				// Command
				else if (strSkim(&data, " ")) {
					outCtl.ch1.SetVoltage(atof(data));
				}
			}
			// CURRent
			else if (strSkim(&data, "CURRent") || strSkim(&data, "CURR")) {
				// Query
				if (strMatch(data, "?")) {
					Return(outCtl.ch1.GetSetCurrent());
				}
				// Command
				else if(strSkim(&data, " ")) {
					outCtl.ch1.SetCurrent(atof(data));
				}
			}
		}
		// SOURce2
		else if (strSkim(&data, "SOURce2:") || strSkim(&data, "SOUR2:")) {
			// VOLTage
			if (strSkim(&data, "VOLTage") || strSkim(&data, "VOLT")) {
				// Query
				if (strMatch(data, "?")) {
					Return(outCtl.ch2.GetSetVoltage());
				}
				// Command
				else if(strSkim(&data, " ")) {
					outCtl.ch2.SetVoltage(atof(data));
				}
			}
			// CURRent
			else if(strSkim(&data, "CURRent") || strSkim(&data, "CURR")) {
				// Query
				if (strMatch(data, "?")) {
					Return(outCtl.ch2.GetSetCurrent());
				}
				// Command
				else if(strSkim(&data, " ")) {
					outCtl.ch2.SetCurrent(atof(data));
				}
			}
		}
		// SOURce3
		else if(strSkim(&data, "SOURce3:") || strSkim(&data, "SOUR3:")) {
			// VOLTage
			if (strSkim(&data, "VOLTage") || strSkim(&data, "VOLT")) {
				// Query
				if (strMatch(data, "?")) {
					Return(outCtl.ch3.GetSetVoltage());
				}
				// Command
				else if(strSkim(&data, " ")) {
					outCtl.ch3.SetVoltage(atof(data));
				}
			}
			// CURRent
			else if(strSkim(&data, "CURRent") || strSkim(&data, "CURR")) {
				// Query
				if (strMatch(data, "?")) {
					Return(outCtl.ch3.GetSetCurrent());
				}
				// Command
				else if(strSkim(&data, " ")) {
					outCtl.ch3.SetCurrent(atof(data));
				}
			}
		}
		// MEASure
		else if(strSkim(&data, "MEASure:") || strSkim(&data, "MEAS:")) {
			// VOLTage
			if (strSkim(&data, "VOLTage? ") || strSkim(&data, "VOLT? ")) {
				if (strSkim(&data, "(@")) {
					while (*data - '0' > 0 && *data - '0' < 9) {
						switch (atoi(data)) {
						case 1:
							Return(outCtl.ch1.GetVoltage());
							break;
						case 2:
							Return(outCtl.ch2.GetVoltage());
							break;
						case 3:
							Return(outCtl.ch3.GetVoltage());
							break;
						default:
							break;
						}
						data++;
						if (*data == ',') {data++; }
					}
				}
			}
			// CURRent
			if (strSkim(&data, "CURRent? ") || strSkim(&data, "CURR? ")) {
				if (strSkim(&data, "(@")) {
					while (*data - '0' > 0 && *data - '0' < 9) {
						switch (atoi(data)) {
						case 1:
							Return(outCtl.ch1.GetCurrent());
							break;
						case 2:
							Return(outCtl.ch2.GetCurrent());
							break;
						case 3:
							Return(outCtl.ch3.GetCurrent());
							break;
						default:
							break;
						}
						data++;
						if (*data == ',') {data++; }
					}
				}
			}
			
		}
		// OUTPut1
		else if(strSkim(&data, "OUTPut1") || strSkim(&data, "OUTP1")) {
			// Query
			if (strMatch(data, "?")) {
				Return(outCtl.ch1.GetState() ? "ON" : "OFF");
			}
			// Command
			else if(strSkim(&data, " ")) {
				if (strMatch(data, "ON")) {outCtl.ch1.SetState(1); }
				else if (strMatch(data, "OFF")) {outCtl.ch1.SetState(0); }
			}
		}
		// OUTPut2
		else if(strSkim(&data, "OUTPut2") || strSkim(&data, "OUTP2")) {
			// Query
			if (strMatch(data, "?")) {
				Return(outCtl.ch2.GetState() ? "ON" : "OFF");
			}
			// Command
			else if(strSkim(&data, " ")) {
				if (strMatch(data, "ON")) {outCtl.ch2.SetState(1); }
				else if (strMatch(data, "OFF")) {outCtl.ch2.SetState(0); }
			}
		}
		// OUTPut3
		else if(strSkim(&data, "OUTPut3") || strSkim(&data, "OUTP3")) {
			// Query
			if (strMatch(data, "?")) {
				Return(outCtl.ch3.GetState() ? "ON" : "OFF");
			}
			// Command
			else if(strSkim(&data, " ")) {
				if (strMatch(data, "ON")) {outCtl.ch3.SetState(1); }
				else if (strMatch(data, "OFF")) {outCtl.ch3.SetState(0); }
			}
		}
		// DISPlay
		else if(strSkim(&data, "DISPlay") || strSkim(&data, "DISP")) {
			// Query
			if (strMatch(data, "?")) {
				Return(reg.DispState ? "ON" : "OFF");
			}
			// Command
			else if(strSkim(&data, " ")) {
				if (strMatch(data, "ON")) {reg.DispState = 1; }
				else if (strMatch(data, "OFF")) {reg.DispState = 0; }
			}
			// Command DATA
			else if(strSkim(&data, ":DATA ")) {
				if (*data == ' ') {reg.DispData[0] = 0;}
				sscanf(data, "%s;\n", reg.DispData);
			}
		}
		
		// Next Command
		if (strchr(data, ';') != NULL) {
			data = strchr(data, ';') + 1;
			goto parseMnemonic;
		}
	}
}

void SCPI_TypeDef::Init() {
	UART_IT_InitTypeDef UARTITInit_Struct;
	
	UARTITInit_Struct.TransferDirection = LL_USART_DIRECTION_TX_RX;
	UARTITInit_Struct.BaudRate = 115200;
	UARTITInit_Struct.DataWidth = LL_USART_DATAWIDTH_8B;
	UARTITInit_Struct.Parity = LL_USART_PARITY_NONE;
	UARTITInit_Struct.StopBits = LL_USART_STOPBITS_1;
	
	uart->Init(&UARTITInit_Struct, SCPI_UARTReceive_Handler, '\n', 1000);
}

void SCPI_TypeDef::Handler() {
	uart->Handler();
}

void SCPI_TypeDef::Return(const char *data) {
	uart->Transmit(data);
}

void SCPI_TypeDef::Return(float data) {
	char str[13];
	uint8_t dec = (uint)abs(data * 100) % 100;
	sprintf(str, "%d.%02d", (int16_t)data, dec);
	uart->Transmit(str);
}

bool SCPI_TypeDef::strMatch(const char *str1, const char *str2) {
	bool s = strncmp(str1, str2, strlen(str2)) == 0;
	return s;
}
bool SCPI_TypeDef::strSkim(char **str1, const char *str2) {
	bool s = strncmp(*str1, str2, strlen(str2)) == 0;
	if (s) {
		*str1 += strlen(str2);
	}
	return s;
}