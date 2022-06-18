#include "scpi.h"
#include "system.h"
#include "outputControl.h"
#include "curveTracer.h"
#include "userInterface.h"
#include <string.h>
#include <ctype.h>

SCPI_TypeDef scpi(&uart1);

void SCPI_UARTReceive_Handler(volatile char *data) {
	scpi.UARTReceive_Handler((char*)data);
}

void SCPI_TypeDef::UARTReceive_Handler(char *data) {
parseMnemonic:
	// SCPI Command Header
	if (strSkim(&data, "*")) {
		// IDN
		if (IsMnemonic(&data, "IDN?")) {
			Return("Convetech,CurveTracer,SCT-2001,v1.00a");
		}
		// RST
		if (IsMnemonic(&data, "RST")) {
			outCtl.DisableAllOutputs();
			outCtl.ch1.SetVoltage(1500);
			outCtl.ch2.SetVoltage(1500);
			outCtl.ch3.SetVoltage(1500);
		}
	}
	// SCPI Instrument Control Header
	else if(strSkim(&data, ":")) {
		// SOURce1
		if (IsMnemonic(&data, "SOURce1:")) {
			// VOLTage
			if (IsMnemonic(&data, "VOLTage")) {
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
			else if(IsMnemonic(&data, "CURRent")) {
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
		else if(IsMnemonic(&data, "SOURce2:")) {
			// VOLTage
			if (IsMnemonic(&data, "VOLTage")) {
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
			else if(IsMnemonic(&data, "CURRent")) {
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
		else if(IsMnemonic(&data, "SOURce3:")) {
			// VOLTage
			if (IsMnemonic(&data, "VOLTage")) {
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
			else if(IsMnemonic(&data, "CURRent")) {
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
		else if(IsMnemonic(&data, "MEASure:")) {
			// VOLTage
			if (IsMnemonic(&data, "VOLTage? ")) {
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
			if (IsMnemonic(&data, "CURRent? ")) {
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
		else if(IsMnemonic(&data, "OUTPut1")) {
			// Query
			if (strMatch(data, "?")) {
				Return(outCtl.ch1.GetState() ? "ON" : "OFF");
			}
			// Command
			else if(strSkim(&data, " ")) {
				if (strMatch(data, "ON")) {outCtl.ch1.SetState(1); }
				else if (strMatch(data, "OFF")) {outCtl.ch1.SetState(0); }
				ui.ForceRedraw();
			}
		}
		// OUTPut2
		else if(IsMnemonic(&data, "OUTPut2")) {
			// Query
			if (strMatch(data, "?")) {
				Return(outCtl.ch2.GetState() ? "ON" : "OFF");
			}
			// Command
			else if(strSkim(&data, " ")) {
				if (strMatch(data, "ON")) {outCtl.ch2.SetState(1); }
				else if (strMatch(data, "OFF")) {outCtl.ch2.SetState(0); }
				ui.ForceRedraw();
			}
		}
		// OUTPut3
		else if(IsMnemonic(&data, "OUTPut3")) {
			// Query
			if (strMatch(data, "?")) {
				Return(outCtl.ch3.GetState() ? "ON" : "OFF");
			}
			// Command
			else if(strSkim(&data, " ")) {
				if (strMatch(data, "ON")) {outCtl.ch3.SetState(1); }
				else if (strMatch(data, "OFF")) {outCtl.ch3.SetState(0); }
				ui.ForceRedraw();
			}
		}
		// TRACe
		else if(IsMnemonic(&data, "TRACe:")) {
			// ROUTe
			if (IsMnemonic(&data, "ROUTe")) {
				if (strSkim(&data, " (@")) {
					Channel_TypeDef *chList[3] = { 0, 0, 0 };
					uint8_t cnt = 0;
					while (*data - '0' > 0 && *data - '0' < 9 && cnt < 3) {
						switch (atoi(data)) {
						case 1:
							chList[cnt++] = &outCtl.ch1;
							break;
						case 2:
							chList[cnt++] = &outCtl.ch2;
							break;
						case 3:
							chList[cnt++] = &outCtl.ch3;
							break;
						default:
							break;
						}
						data++;
						if (*data == ',') {data++; }
					}
					curveTracer.SetupChannel(chList[0], chList[1], chList[2]);
				}
			}
			// VOLTage
			else if(IsMnemonic(&data, "VOLTage:")) {
				// STARt
				if (IsMnemonic(&data, "STARt")) {
					// Command
					if (strSkim(&data, " ")) {
						curveTracer.vStart = atof(data);
					}
					// Query
					else if(strSkim(&data, "?")) {
						Return(curveTracer.vStart);	
					}
				}
				// STOP
				if (IsMnemonic(&data, "STOP")) {
					// Command
					if (strSkim(&data, " ")) {
						curveTracer.vEnd = atof(data);
					}
					// Query
					else if(strSkim(&data, "?")) {
						Return(curveTracer.vEnd);	
					}
				}
				// STEP
				if (IsMnemonic(&data, "STEP")) {
					// Command
					if (strSkim(&data, " ")) {
						curveTracer.vStep = atof(data);
					}
					// Query
					else if(strSkim(&data, "?")) {
						Return(curveTracer.vStep);	
					}
				}
			}
			// CURRent
			else if(IsMnemonic(&data, "CURRent:")) {
				// STARt
				if (IsMnemonic(&data, "STARt")) {
					// Command
					if (strSkim(&data, " ")) {
						curveTracer.iStart = atof(data);
					}
					// Query
					else if(strSkim(&data, "?")) {
						Return(curveTracer.iStart);	
					}
				}
				// STOP
				if (IsMnemonic(&data, "STOP")) {
					// Command
					if (strSkim(&data, " ")) {
						curveTracer.iEnd = atof(data);
					}
					// Query
					else if(strSkim(&data, "?")) {
						Return(curveTracer.iEnd);	
					}
				}
				// STEP
				if (IsMnemonic(&data, "STEP")) {
					// Command
					if (strSkim(&data, " ")) {
						curveTracer.iStep = atof(data);
					}
					// Query
					else if(strSkim(&data, "?")) {
						Return(curveTracer.iStep);	
					}
				}
				// RANGe
				if (IsMnemonic(&data, "RANGe")) {
					// Command
					if (strSkim(&data, " ")) {
						curveTracer.iLim = atof(data);
					}
					// Query
					else if(strSkim(&data, "?")) {
						Return(curveTracer.iLim);	
					}
				}
			}
			// STATe
			else if(IsMnemonic(&data, "STATe")) {
				// INITiate
				if(IsMnemonic(&data, ":INITiate")) {
					if (!curveTracer.IsSampling()) {
						curveTracer.tSample = 100;
						curveTracer.Start();
						ui.SetScreenMenu(2);
						ui.ForceRedraw();
					}
				}
				// STOP
				else if (IsMnemonic(&data, ":STOP")) {
					if (curveTracer.IsSampling()) {
						curveTracer.Stop();
						ui.ForceRedraw();
					}
				}
				// Query
				else if(strSkim(&data, "?")) {
					Return(curveTracer.IsSampling());
				}
			}
			// INVerted
			else if(IsMnemonic(&data, "INVerted")) {
				// ON
				if (IsMnemonic(&data, " ON")) {
					outCtl.InvertChannels(1);
				}
				// OFF
				if (IsMnemonic(&data, " OFF")) {
					outCtl.InvertChannels(0);
				}
				// Query
				else if(strSkim(&data, "?")) {
					Return(outCtl.IsInverted());
				}
			}
		}
		// DISPlay
		else if(IsMnemonic(&data, "DISPlay")) {
			IsMnemonic(&data, ":STATe");
			// Query
			if (strMatch(data, "?")) {
				Return(reg.DispState ? "ON" : "OFF");
			}
			// STATe
			else if(strSkim(&data, " ")) {
				if (strMatch(data, "ON")) {reg.DispState = 1; }
				else if (strMatch(data, "OFF")) {reg.DispState = 0; }
			}
			// DATA
			else if(strSkim(&data, ":DATA ")) {
				if (*data == ' ') {reg.DispData[0] = 0;}
				sscanf(data, "%s;\n", reg.DispData);
			}
			// MENU
			else if(strSkim(&data, ":MENU ")) {
				ui.SetScreenMenu(atoi(data));
			}
			// CLEar
			else if(strSkim(&data, ":CLEar")) {
				ui.ForceRedraw();
			}
		}
		// STATus
		if (IsMnemonic(&data, "STATus:")) {
			// SETTling
			if (IsMnemonic(&data, "SETTling?")) {
				Return(!outCtl.ch1.IsStable() || !outCtl.ch2.IsStable() || !outCtl.ch3.IsStable());	
			}
			// VOLTage
			if (IsMnemonic(&data, "VOLTage?")) {
				Return((int)sys.ReadVsenseVin());
			}
			// TEMPerature
			if (IsMnemonic(&data, "TEMPerature?")) {
				Return(sys.ReadDriverTemp());
			}
			// SWEeping
			if (IsMnemonic(&data, "SWEeping?")) {
				Return(curveTracer.IsSampling());
			}
		}
		// SYSTem
		else if(IsMnemonic(&data, "SYSTem:")) {
			// BEEPer
			if (IsMnemonic(&data, "BEEPer")) {
				IsMnemonic(&data, ":STATe");
				// TIME
				if (IsMnemonic(&data, ":TIME ")) {
					reg.BeepTime = atoi(data);
				}
				// COUNt
				else if(IsMnemonic(&data, ":COUNt ")) {
					reg.BeepCount = atoi(data);
				}
				// STATe
				else if (strSkim(&data, " ON")) {
					ui.Beep(reg.BeepTime, reg.BeepCount);
				}
			}
		}
		
		// Parse Next Command
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
	if (sys.IsUSBConnected()) {
		if (curveTracer.IsSamplingDone() && curveTracer.IsNewSample()) {
			Return("TRAC STOP");
		}
		if (curveTracer.IsSampling()) {
			char buff[51];
			uint16_t sample = curveTracer.GetSampleCount();
			if (curveTracer.IsNewSample() && sample > 0) {
				if (sample == 1 && curveTracer.GetSequenceCount() == 0) {
					sprintf(buff, "TRAC S%d", curveTracer.GetSequenceCount());
					Return(buff);
				}
				uint8_t vDec = (uint)abs(curveTracer.data[sample - 1].v * 100) % 100;
				uint8_t iDec = (uint)abs(curveTracer.data[sample - 1].i * 100) % 100;
				sprintf(buff, "P%d V%d.%02d,I%d.%02d", sample, (int)curveTracer.data[sample - 1].v, vDec, (int)curveTracer.data[sample - 1].i, iDec);
				Return(buff);
			}
			if (curveTracer.IsNewSequence() && curveTracer.IsNewSample()) {
				sprintf(buff, "TRAC S%d", curveTracer.GetSequenceCount());
				Return(buff);
			}
		}
	}
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
void SCPI_TypeDef::Return(int data) {
	char str[13];
	sprintf(str, "%d", data);
	uart->Transmit(str);
}

bool SCPI_TypeDef::IsMnemonic(char **str, const char *m) {
	char abr[11];
	uint8_t abrl = 0;
	for (uint8_t i = 0; m[i]; i++) {
		if (!islower(m[i])) {
			abr[abrl++] = m[i];
		}
	}
	abr[abrl] = 0;
	
	return strSkim(str, m) || strSkim(str, abr);
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