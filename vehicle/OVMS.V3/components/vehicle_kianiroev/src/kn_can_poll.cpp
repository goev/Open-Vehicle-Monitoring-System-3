/*
;    Project:       Open Vehicle Monitor System
;    Date:          21th January 2019
;
;    (C) 2019       Geir Øyvind Vælidalo <geir@validalo.net>
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
; THE SOFTWARE.
*/
#include "vehicle_kianiroev.h"

static const char *TAG = "v-kianiroev";

/**
 * Incoming poll reply messages
 */
void OvmsVehicleKiaNiroEv::IncomingPollReply(canbus* bus, uint16_t type, uint16_t pid, uint8_t* data, uint8_t length, uint16_t mlremain)
  {
	//ESP_LOGW(TAG, "%03x TYPE:%x PID:%02x %x %02x %02x %02x %02x %02x %02x %02x %02x", m_poll_moduleid_low, type, pid, length, data[0], data[1], data[2], data[3],
	//	data[4], data[5], data[6], data[7]);
	switch (m_poll_moduleid_low)
		{
		// ****** SJB *****
		case 0x779:
			IncomingSJB(bus, type, pid, data, length, mlremain);
			break;

		// ****** OBC ******
		case 0x79c:
			IncomingOBC(bus, type, pid, data, length, mlremain);
			break;

	  // ****** HVAC ******
	  case 0x7b3:
				IncomingHVAC(bus, type, pid, data, length, mlremain);
				break;

	   // ****** TPMS ******
		case 0x7de:
			IncomingTPMS(bus, type, pid, data, length, mlremain);
			break;

		// ******* VMCU ******
		case 0x7ea:
			IncomingVMCU(bus, type, pid, data, length, mlremain);
			break;

		// ***** BMC ****
		case 0x7ec:
			IncomingBMC(bus, type, pid, data, length, mlremain);
			break;

		// ***** LDC ****
		case 0x7cd:
			IncomingLDC(bus, type, pid, data, length, mlremain);
			break;

		default:
			ESP_LOGD(TAG, "Unknown module: %03x", m_poll_moduleid_low);
			break;
	  }
  }

/**
 * Handle incoming messages from HVAC poll.
 */
void OvmsVehicleKiaNiroEv::IncomingHVAC(canbus* bus, uint16_t type, uint16_t pid, uint8_t* data, uint8_t length, uint16_t mlremain)
	{
	switch (pid)
		{
		case 0x00:
			if (m_poll_ml_frame == 1)
				{
				//bVal = CAN_BYTE(1)/2-40; Indoor TEMP
				//bVal = CAN_BYTE(2)/2-40; Outdoor TEMP
				StdMetrics.ms_v_env_temp->SetValue(TO_CELCIUS(CAN_BYTE(2)/2.0), Celcius);
				StdMetrics.ms_v_env_cabintemp->SetValue(TO_CELCIUS(CAN_BYTE(1)/2.0), Celcius);
				}
			break;
		}
	}

/**
 * Handle incoming messages from TPMS poll.
 */
void OvmsVehicleKiaNiroEv::IncomingTPMS(canbus* bus, uint16_t type, uint16_t pid, uint8_t* data, uint8_t length, uint16_t mlremain)
	{
	uint8_t bVal;
	uint32_t lVal;

	switch (pid)
		{
		case 0x06:
			if (m_poll_ml_frame == 0)
				{
				lVal = CAN_UINT32(4);
				SET_TPMS_ID(0, lVal);
				}
			else if (m_poll_ml_frame == 1)
				{
				bVal = CAN_BYTE(0);
				if (bVal > 0) StdMetrics.ms_v_tpms_fl_p->SetValue( TO_PSI(bVal), PSI);
				StdMetrics.ms_v_tpms_fl_t->SetValue( TO_CELCIUS(CAN_BYTE(1)), Celcius);
				lVal = (kn_tpms_id[1] & 0x000000ff) | (CAN_UINT32(4) & 0xffffff00);
				SET_TPMS_ID(1, lVal);
				}
			else if (m_poll_ml_frame == 2)
				{
				lVal = (uint32_t) CAN_BYTE(0) | (kn_tpms_id[1] & 0xffffff00);
				SET_TPMS_ID(1, lVal);
				bVal = CAN_BYTE(1);
				if (bVal > 0) StdMetrics.ms_v_tpms_fr_p->SetValue( TO_PSI(bVal), PSI);
				StdMetrics.ms_v_tpms_fr_t->SetValue( TO_CELCIUS(CAN_BYTE(2)), Celcius);
				lVal = (kn_tpms_id[2] & 0x0000ffff) | (CAN_UINT32(5) & 0xffff0000);
				SET_TPMS_ID(2, lVal);

				}
			else if (m_poll_ml_frame == 3)
				{
				lVal = ((uint32_t) CAN_UINT(0)) | (kn_tpms_id[2] & 0xffff0000);
				SET_TPMS_ID(2, lVal);
				bVal = CAN_BYTE(2);
				if (bVal > 0) StdMetrics.ms_v_tpms_rl_p->SetValue( TO_PSI(bVal), PSI);
				StdMetrics.ms_v_tpms_rl_t->SetValue( TO_CELCIUS(CAN_BYTE(3)), Celcius);
				lVal = (kn_tpms_id[3] & 0x00ffffff) | ((uint32_t) CAN_BYTE(6) << 24);
				SET_TPMS_ID(3, lVal);

			} else if (m_poll_ml_frame == 4) {
				lVal = (CAN_UINT24(0)) | (kn_tpms_id[3] & 0xff000000);
				SET_TPMS_ID(3, lVal);
				bVal = CAN_BYTE(3);
				if (bVal > 0) StdMetrics.ms_v_tpms_rr_p->SetValue( TO_PSI(bVal), PSI);
				StdMetrics.ms_v_tpms_rr_t->SetValue( TO_CELCIUS(CAN_BYTE(4)), Celcius);
			}
			break;
		}
	}

/**
 * Handle incoming messages from On Board Charger-poll
 *
 * - OBC-voltage
 * - Pilot signal duty cycle
 * - Charger temperature
 */
void OvmsVehicleKiaNiroEv::IncomingOBC(canbus* bus, uint16_t type, uint16_t pid, uint8_t* data, uint8_t length, uint16_t mlremain)
	{
	switch (pid)
		{
		case 0x02:
			if (m_poll_ml_frame == 1)
				{
				kn_obc_volt = (float) CAN_UINT(2) / 10.0;
				//} else if (vehicle_poll_ml_frame == 2) {
				//kn_obc_ampere = ((UINT) can_databuffer[4 + CAN_ADJ] << 8)
				//        | (UINT) can_databuffer[5 + CAN_ADJ];
				}
			else if (m_poll_ml_frame == 2)
				{
				m_obc_pilot_duty->SetValue( (float) CAN_BYTE(6) / 3.0 );
				}
			else if (m_poll_ml_frame == 3)
				{
				StdMetrics.ms_v_charge_temp->SetValue( (float) (CAN_BYTE(0)+CAN_BYTE(1)+CAN_BYTE(2))/3, Celcius );
				}
			break;
		}
	}

/**
 * Handle incoming messages from VMCU-poll
 *
 * - Gear shifter position
 * - VIN
 * - RPM
 * - Motor temperature
 * - Inverter temperature
 */
void OvmsVehicleKiaNiroEv::IncomingVMCU(canbus* bus, uint16_t type, uint16_t pid, uint8_t* data, uint8_t length, uint16_t mlremain)
	{
	INT base;
	uint8_t bVal;

	switch (pid)
		{
		case 0x00:
			if (m_poll_ml_frame == 1)
				{
				//TODO kn_shift_bits.value = CAN_BYTE(3);
				}
			break;

		case 0x02:
			// VIN (multi-line response):
			if (type == VEHICLE_POLL_TYPE_OBDIIVEHICLE)
				{
				base = m_poll_ml_offset - length;

				for (bVal = 0; (bVal < length) && ((base + bVal)<sizeof (m_vin)); bVal++)
					if(base+bVal>0) m_vin[base + bVal-1] = CAN_BYTE(bVal);
				if (m_poll_ml_remain == 0 && base+bVal>=0) m_vin[base + bVal] = 0;

				//Set VIN
				StandardMetrics.ms_v_vin->SetValue(m_vin);
				}
			if (type == VEHICLE_POLL_TYPE_OBDIIGROUP)
				{
				if (m_poll_ml_frame == 1)
					{
					//StdMetrics.ms_v_mot_rpm->SetValue( ((uint16_t)CAN_BYTE(5)<<8) | (uint16_t)CAN_BYTE(6) );
					}
				else if (m_poll_ml_frame == 3)
					{
					//StdMetrics.ms_v_mot_temp->SetValue( TO_CELCIUS(CAN_BYTE(4)), Celcius);
					//StdMetrics.ms_v_inv_temp->SetValue( TO_CELCIUS(CAN_BYTE(5)), Celcius );

					}
				}
			break;
		}
	}

/**
 * Handle incoming messages from BMC-poll
 *
 * - Pilot signal available
 * - CCS / Type2
 * - Battery current
 * - Battery voltage
 * - Battery module temp 1-8
 * - Cell voltage max / min + cell #
 * + more
 */
void OvmsVehicleKiaNiroEv::IncomingBMC(canbus* bus, uint16_t type, uint16_t pid, uint8_t* data, uint8_t length, uint16_t mlremain)
	{
	uint8_t bVal;

	if (type == VEHICLE_POLL_TYPE_OBDIIGROUP)
		{
		switch (pid)
			{
			case 0x02:
				{
				if (m_poll_ml_frame == 3)
					{
					m_b_aux_soc->SetValue( CAN_BYTE(5), Percentage);
					StdMetrics.ms_v_bat_12v_current->SetValue ((float)CAN_INT(3)/1000.0 , Ampere);
					//TODO m_b_aux_current->SetValue( CAN_INT(3)/1000, Percentage);
					//TODO m_b_aux_voltage->SetValue( CAN_INT(1)/1000, Percentage);
					}
				}
				break;
			}
		}

	else if (type == VEHICLE_POLL_TYPE_OBDII_2201)
		{
		switch (pid)
			{
			case 0x01:
				// diag page 01: skip first frame (no data)
				if (m_poll_ml_frame > 0) {
					if (m_poll_ml_frame == 1)
						{
						m_c_power->SetValue( (float)CAN_UINT(2)/100.0, kW);
						bVal = CAN_BYTE(6);
						StdMetrics.ms_v_charge_pilot->SetValue((bVal & 0x80) > 0);
						kn_charge_bits.ChargingCCS = ((bVal & 0x40) > 0);
						kn_charge_bits.ChargingType2 = ((bVal & 0x20) > 0);

						}
					else if (m_poll_ml_frame == 2)
						{
						kn_battery_current = CAN_UINT(0);
						StdMetrics.ms_v_bat_current->SetValue((float)kn_battery_current/10.0, Amps);
						StdMetrics.ms_v_bat_voltage->SetValue((float)CAN_UINT(2)/10.0, Volts);
						m_b_min_temperature->SetValue( CAN_BYTE(5) );
						m_b_max_temperature->SetValue( CAN_BYTE(4) );
						BmsRestartCellTemperatures();
						BmsSetCellTemperature(0, CAN_BYTE(6));
						StdMetrics.ms_v_bat_temp->SetValue((float)CAN_BYTE(5), Celcius); //TODO Should we use Min temp or Max Temp?
						}
					else if (m_poll_ml_frame == 3)
						{
						 BmsSetCellTemperature(1, CAN_BYTE(0));
						 BmsSetCellTemperature(2, CAN_BYTE(1));
						 BmsSetCellTemperature(3, CAN_BYTE(2));
						BmsSetCellTemperature(4, CAN_BYTE(3));
							m_b_inlet_temperature->SetValue( CAN_BYTE(5) );
						m_b_cell_volt_max->SetValue((float)CAN_BYTE(6)/50.0, Volts);
						}
					else if (m_poll_ml_frame == 4)
						{
						m_b_cell_volt_max_no->SetValue(CAN_BYTE(0));
						m_b_cell_volt_min->SetValue((float)CAN_BYTE(1)/50.0, Volts);
						m_b_cell_volt_min_no->SetValue(CAN_BYTE(2));
						kn_battery_fan_feedback = CAN_BYTE(3); //TODO Battery fan feedback
						kn_charge_bits.FanStatus = CAN_BYTE(4) & 0xF;  //TODO Battery fan speed
						StdMetrics.ms_v_bat_12v_voltage->SetValue ((float)CAN_BYTE(5)/10.0 , Volts);
						kn_battery_cum_charge_current = (kn_battery_cum_charge_current & 0x00FFFFFF) | ((uint32_t) CAN_UINT(6) << 24);
						}
					else if (m_poll_ml_frame == 5)
						{
						kn_battery_cum_charge_current = (kn_battery_cum_charge_current & 0xFF000000) | ((uint32_t) CAN_UINT24(0));
						kn_battery_cum_discharge_current = CAN_UINT32(3);

						}
					else if (m_poll_ml_frame == 6)
						{
						kn_battery_cum_charge = CAN_UINT32(0);
						kn_battery_cum_discharge = (kn_battery_cum_discharge & 0xFF) | ((uint32_t) CAN_UINT24(4) << 8);
						}
					else if (m_poll_ml_frame == 7)
						{
						kn_battery_cum_discharge = (kn_battery_cum_discharge & 0xFFFFFF00) | ((uint32_t) CAN_BYTE(0));
						kn_battery_cum_op_time = CAN_UINT32(1) / 3600;
						}
					}
				break;

			case 0x02: //Cell voltages
			case 0x03:
			case 0x04:
				// diag page 02-04: skip first frame (no data)
				if(m_poll_ml_frame==0 && pid==2) BmsRestartCellVoltages();
				if(m_poll_ml_frame>0)
					{
					base = ((pid-2)*32) - 1 + (m_poll_ml_frame-1) * 7;
					bVal=0;
					if(m_poll_ml_frame==1)
						{
						bVal++;
						}
					for (;bVal < length && ((base + bVal)<96); bVal++)
						{
						//ESP_LOGI(TAG, "[%d %d] = %f", pid, base+bVal, (float)CAN_BYTE(bVal) * 0.02);
						BmsSetCellVoltage((uint8_t)(base + bVal), (float)CAN_BYTE(bVal) * 0.02);
						}
					}
				break;
			case 0x05:
				if (m_poll_ml_frame == 3)
					{
					//kn_air_bag_hwire_duty = can_databuffer[5 + CAN_ADJ];
					m_b_heat_1_temperature->SetValue( CAN_BYTE(6) );
					}
				else if (m_poll_ml_frame == 4)
					{
					m_b_heat_2_temperature->SetValue( CAN_BYTE(0) );
					m_b_cell_det_max->SetValue( (float)CAN_UINT(1)/10.0 );
					m_b_cell_det_max_no->SetValue( CAN_BYTE(3) );
					m_b_cell_det_min->SetValue( (float)CAN_UINT(4)/10.0 );
					m_b_cell_det_min_no->SetValue( CAN_BYTE(6) );
					}
				else if (m_poll_ml_frame == 5)
					{
					//TODO CAN_BYTE(0) Display SOC
					BmsSetCellVoltage(96, (float)CAN_BYTE(3) * 0.02);
					BmsSetCellVoltage(97, (float)CAN_BYTE(4) * 0.02);
					}
				break;
			}
		}
	}

/**
 * Handle incoming messages from LDC-poll
 *
 * - LDC out voltage
 * - LDC out current
 * - LDC in voltage
 * - LDC temperature
 */
void OvmsVehicleKiaNiroEv::IncomingLDC(canbus* bus, uint16_t type, uint16_t pid, uint8_t* data, uint8_t length, uint16_t mlremain)
	{
	switch (pid)
		{
		case 0x01:
			// 12V system
			kn_ldc_enabled = (CAN_BYTE(0) & 0x07) != 0;
			if( CAN_BYTE(0)==0x07 )
				{
				m_ldc_out_voltage->SetValue( CAN_BYTE(1) / 10.0 );
				m_ldc_out_current->SetValue( CAN_BYTE(2) );
				m_ldc_in_voltage->SetValue( CAN_BYTE(3) * 2 );
				m_ldc_temperature->SetValue( CAN_BYTE(4) - 100 );
				}
			break;
		}
	}

/**
 * Handle incoming messages from SJB-poll
 */
void OvmsVehicleKiaNiroEv::IncomingSJB(canbus* bus, uint16_t type, uint16_t pid, uint8_t* data, uint8_t length, uint16_t mlremain)
	{
	//ESP_LOGW(TAG, "779X %02x %02x %02x %02x %02x %02x %02x %02x %02x", pid, m_poll_ml_frame, CAN_BYTE(0), CAN_BYTE(1), CAN_BYTE(2), CAN_BYTE(3), CAN_BYTE(4), CAN_BYTE(5), CAN_BYTE(6));
	switch (pid)
		{
		case 0x04:
			if (m_poll_ml_frame == 1)
				{
				//ESP_LOGW(TAG, "779 8 21 %02x %02x %02x %02x %02x %02x %02x", CAN_BYTE(0), CAN_BYTE(1), CAN_BYTE(2), CAN_BYTE(3), CAN_BYTE(4), CAN_BYTE(5), CAN_BYTE(6));
				}
			break;
		}
	}