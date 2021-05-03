from micropython import const
import ustruct

HTS_CTRL_REG1_PD = const(0x1 << 7)
HTS_CTRL_REG1_BUD = const(0x1 << 2)
HTS_CTRL_REG2_BOOT = const(0x1 << 7)
HTS_CTRL_REG2_ONE_SHOT = const(0x1 << 0)


_WHO_AM_I = const(0x0F)

_CTRL_REG1 = const(0x20)
_CTRL_REG2 = const(0x21)
_CTRL_REG3 = const(0x22)
_STATUS_REG = const(0x27)
# some addresses are anded to set the  top bit so that multi-byte reads will work
_HUMIDITY_OUT_L = const(0x28 | 0x80)  # Humidity output register (LSByte)
_TEMP_OUT_L = const(0x2A | 0x80)  # Temperature output register (LSByte)

_H0_RH_X2 = const(0x30)  # Humididy calibration LSB values
_H1_RH_X2 = const(0x31)  # Humididy calibration LSB values

_T0_DEGC_X8 = const(0x32)  # First byte of T0, T1 calibration values
_T1_DEGC_X8 = const(0x33)  # First byte of T0, T1 calibration values
_T1_T0_MSB = const(0x35)  # Top 2 bits of T0 and T1 (each are 10 bits)

_H0_T0_OUT = const(0x36 | 0x80)  # Humididy calibration Time 0 value
_H1_T1_OUT = const(0x3A | 0x80)  # Humididy calibration Time 1 value

_T0_OUT = const(0x3C | 0x80)  # T0_OUT LSByte
_T1_OUT = const(0x3E | 0x80)  # T1_OUT LSByte

_HTS221_CHIP_ID = 0xBC
_HTS221_DEFAULT_ADDRESS = 0x5F


class HTS:
    def __init__(self, i2c):
        self.i2c = i2c
        self.servants = i2c.scan()
        print('Servant addrs: ' + str(self.servants))

        if not self.servants:
            raise RuntimeError("Failed to find any I2C devices")

        self.i2c.start()
        # check chip ID is correct
        chip_id = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _WHO_AM_I, 1)
        print(str(chip_id))
        if _HTS221_CHIP_ID not in [chip_id]:
            print('Expected ' + str(_HTS221_CHIP_ID))
            print('Recieved ' + str(chip_id))
            raise RuntimeError("Failed to init HTS221")
        self.i2c.stop()

        self.set_enable_and_block_update()
        self.calibrate()

        print('Initialized HTS221.')

    def read_from_mem_int(self, main_addr, sub_addr, num_bytes, signed=False):
        # TODO - signed?
        b_data = self.i2c.readfrom_mem(main_addr, sub_addr, num_bytes)
        # print("Addr: " + str(sub_addr))
        # print("Read: " + str(b_data))
        if signed:
            ret_int = ustruct.unpack("<h", b_data)[0]
            # print("Signed Ret Int: " + str(ret_int))
            return ret_int
        else:
            ret_int = int.from_bytes(b_data, 'big')
            # print("Signed Ret Int: " + str(ret_int))
            return ret_int

    def write_to_mem_int(self, main_addr, sub_addr, out):
        return self.i2c.writeto_mem(main_addr, sub_addr, out.to_bytes(1, 'big', False))

    @staticmethod
    def bytes_to_int(bytes):
        result = 0
        for b in bytes:
            result = result * 256 + int(b)
        return result

    @staticmethod
    def int_to_bytes(value, length):
        result = []
        for i in range(0, length):
            result.append(value >> (i * 8) & 0xff)
        result.reverse()
        return result

    def set_enable_and_block_update(self):
        self.i2c.start()

        self.restart()

        reg1 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _CTRL_REG1, 1)
        # set enable pin 7th bit (from 0)
        reg1 |= HTS_CTRL_REG1_PD
        reg1 |= HTS_CTRL_REG1_BUD
        self.write_to_mem_int(_HTS221_DEFAULT_ADDRESS, _CTRL_REG1, reg1)

        self.i2c.stop()

    def calibrate(self):
        self.i2c.start()

        print("HTS CAL")

        # print("Addr: _T1_T0_MSB")
        T1_T0_MSB = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _T1_T0_MSB, 1)
        # print("Addr: _T0_DEGC_X8")
        self.calib_temp_value_0 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _T0_DEGC_X8, 1)
        self.calib_temp_value_0 |= (T1_T0_MSB & 0b0011) << 8

        # print("Addr: _T1_DEGC_X8")
        self.calibrated_value_1 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _T1_DEGC_X8, 1)
        self.calibrated_value_1 |= (T1_T0_MSB & 0b1100) << 6

        self.calib_temp_value_0 >>= 3  # divide by 8 to remove x8
        self.calibrated_value_1 >>= 3  # divide by 8 to remove x8

        # print("Addr: _T0_OUT +1")
        self.calib_temp_meas_0 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _T0_OUT, 2, signed=True)
        # print("Addr: _T1_OUT +1")
        self.calib_temp_meas_1 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _T1_OUT, 2, signed=True)

        # print("Addr: _H0_RH_X2")
        self.calib_hum_value_0 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _H0_RH_X2, 1)
        self.calib_hum_value_0 >>= 1  # divide by 2 to remove x2

        # print("Addr: _H1_RH_X2")
        self.calib_hum_value_1 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _H1_RH_X2, 1)
        self.calib_hum_value_1 >>= 1  # divide by 2 to remove x2

        # print("Addr: _H0_T0_OUT + 1")
        self.calib_hum_meas_0 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _H0_T0_OUT, 2, signed=True)
        # print("Addr: _H1_T1_OUT + 1")
        self.calib_hum_meas_1 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _H1_T1_OUT, 2, signed=True)

        self.i2c.stop()

    def get_temp_humid_measurements(self):
        # sets relative_humidity + temperature properties
        self.i2c.start()

        # first one shot
        reg2 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _CTRL_REG2, 1)
        reg2 |= HTS_CTRL_REG2_ONE_SHOT
        self.write_to_mem_int(_HTS221_DEFAULT_ADDRESS, _CTRL_REG2, reg2)

        reg2 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _CTRL_REG2, 1)
        while reg2 & HTS_CTRL_REG2_ONE_SHOT:
            # while boot is 1
            reg2 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _CTRL_REG2, 1)

        # then read regs
        # print("Addr: _HUMIDITY_OUT_L + 1")
        self._raw_humidity = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _HUMIDITY_OUT_L, 2, signed=True)
        # print("Addr: _TEMP_OUT_L + 1")
        self._raw_temperature = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _TEMP_OUT_L, 2, signed=True)

        self.i2c.stop()


    @property
    def relative_humidity(self):
        """The current relative humidity measurement in %rH"""
        calibrated_value_delta = self.calib_hum_value_1 - self.calib_hum_value_0
        calibrated_measurement_delta = self.calib_hum_meas_1 - self.calib_hum_meas_0

        calibration_value_offset = self.calib_hum_value_0
        calibrated_measurement_offset = self.calib_hum_meas_0
        zeroed_measured_humidity = self._raw_humidity - calibrated_measurement_offset

        correction_factor = calibrated_value_delta / calibrated_measurement_delta

        adjusted_humidity = (
                zeroed_measured_humidity * correction_factor + calibration_value_offset
        )

        return adjusted_humidity

    @property
    def temperature(self):
        """The current temperature measurement in degrees C"""

        calibrated_value_delta = self.calibrated_value_1 - self.calib_temp_value_0
        calibrated_measurement_delta = self.calib_temp_meas_1 - self.calib_temp_meas_0

        calibration_value_offset = self.calib_temp_value_0
        calibrated_measurement_offset = self.calib_temp_meas_0
        zeroed_measured_temp = self._raw_temperature - calibrated_measurement_offset

        correction_factor = calibrated_value_delta / calibrated_measurement_delta

        adjusted_temp = (
                                zeroed_measured_temp * correction_factor
                        ) + calibration_value_offset

        return adjusted_temp

    def restart(self):
        # set boot bit and wait until 0
        self.i2c.start()

        reg2 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _CTRL_REG2, 1)
        reg2 |= HTS_CTRL_REG2_BOOT
        self.write_to_mem_int(_HTS221_DEFAULT_ADDRESS, _CTRL_REG2, reg2)

        reg2 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _CTRL_REG2, 1)
        while reg2 & HTS_CTRL_REG2_BOOT:
            # while boot is 1
            reg2 = self.read_from_mem_int(_HTS221_DEFAULT_ADDRESS, _CTRL_REG2, 1)

        self.i2c.stop()

    @staticmethod
    def __celsius_to_fahrenheit(temp):
        return temp * 9 / 5 + 32

    @staticmethod
    def __2bytes_to_int(data):
        # Int range of any register: [-32768, +32767]
        # Must determine signing of int
        if not data[0] & 0x80:
            return data[0] << 8 | data[1]
        return -(((data[0] ^ 0xFF) << 8) | (data[1] ^ 0xFF) + 1)

