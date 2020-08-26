#include <stdio.h>
#include "Mcp22xx_GetErrorName.h"

char* Mcp2210_GetErrorName(int ErrorCode)
{
	switch (ErrorCode)
	{
		{
	case 0:		return "E_SUCCESS";							case -1: return "E_ERR_UNKOWN_ERROR";            case -2: return "E_ERR_INVALID_PARAMETER";
	case -10:	return "E_ERR_NULL";                        case -20: return "E_ERR_MALLOC";                 case -30: return "E_ERR_INVALID_HANDLE_VALUE";
	case -100:	return "E_ERR_FIND_DEV";                    case -101: return "E_ERR_NO_SUCH_INDEX";         case -103: return "E_ERR_DEVICE_NOT_FOUND";
	case -104:	return "E_ERR_INTERNAL_BUFFER_TOO_SMALL";   case -105: return "E_ERR_OPEN_DEVICE_ERROR";     case -106: return "E_ERR_CONNECTION_ALREADY_OPENED";
	case -107:	return "E_ERR_CLOSE_FAILED";                case -108: return "E_ERR_NO_SUCH_SERIALNR";      case -110: return "E_ERR_HID_RW_TIMEOUT";
	case -111:	return "E_ERR_HID_RW_FILEIO";               case -200: return "E_ERR_CMD_FAILED";            case -201: return "E_ERR_CMD_ECHO";
	case -202:	return "E_ERR_SUBCMD_ECHO";                 case -203: return "E_ERR_SPI_CFG_ABORT";         case -204: return "E_ERR_SPI_EXTERN_MASTER";
	case -205:	return "E_ERR_SPI_TIMEOUT";                 case -206: return "E_ERR_SPI_RX_INCOMPLETE";     case -207: return "E_ERR_SPI_XFER_ONGOING";
	case -300:	return "E_ERR_BLOCKED_ACCESS";              case -301: return "E_ERR_EEPROM_WRITE_FAIL";     case -350: return "E_ERR_NVRAM_LOCKED";
	case -351:	return "E_ERR_WRONG_PASSWD";                case -352: return "E_ERR_ACCESS_DENIED";         case -353: return "E_ERR_NVRAM_PROTECTED";
	case -354:	return "E_ERR_PASSWD_CHANGE";               case -400: return "E_ERR_STRING_DESCRIPTOR";     case -401: return "E_ERR_STRING_TOO_LARGE";
	default: return "No Equivalent Error Code";
		}
	}
}

char* Mcp2221_GetErrorName(int ErrorCode)
{
	switch (ErrorCode)
	{
		/********************************
		Error codes
		*********************************/
	case 0: return "E_NO_ERR";
	case -1: return "E_ERR_UNKOWN_ERROR";
	case -2: return "E_ERR_CMD_FAILED";
	case -3: return "E_ERR_INVALID_HANDLE";
	case -4: return "E_ERR_INVALID_PARAMETER";
	case -5: return "E_ERR_INVALID_PASS";
	case -6: return "E_ERR_PASSWORD_LIMIT_REACHED";
	case -7: return "E_ERR_FLASH_WRITE_PROTECTED";

		// null pointer received
	case -10: return "E_ERR_NULL";

		// destination error_string too small
	case -11: return "E_ERR_DESTINATION_TOO_SMALL";
	case -12: return "E_ERR_INPUT_TOO_LARGE";
	case -13: return "E_ERR_FLASH_WRITE_FAILED";
	case -14: return "E_ERR_MALLOC";

		//we tried to connect to a device with a non existent index
	case -101: return "E_ERR_NO_SUCH_INDEX";

		// no device matching the provided criteria was found
	case -103: return "E_ERR_DEVICE_NOT_FOUND";

		// one of the internal buffers of the function was too small
	case -104: return "E_ERR_INTERNAL_BUFFER_TOO_SMALL";

		// an error occurred when trying to get the device handle
	case -105: return "E_ERR_OPEN_DEVICE_ERROR";

		// connection already opened
	case -106: return "E_ERR_CONNECTION_ALREADY_OPENED";
	case -107: return "E_ERR_CLOSE_FAILED";


		/******* I2C errors *******/
	case -401: return "E_ERR_INVALID_SPEED";
	case -402: return "E_ERR_SPEED_NOT_SET";
	case -403: return "E_ERR_INVALID_BYTE_NUMBER";
	case -404: return "E_ERR_INVALID_ADDRESS";
	case -405: return "E_ERR_I2C_BUSY";

		//mcp2221 signaled an error during the i2c read operation
	case -406: return "E_ERR_I2C_READ_ERROR";
	case -407: return "E_ERR_ADDRESS_NACK";
	case -408: return "E_ERR_TIMEOUT";
	case -409: return "E_ERR_TOO_MANY_RX_BYTES";

		//could not copy the data received from the slave into the provided buffer;
	case -410: return "E_ERR_COPY_RX_DATA_FAILED";

		// failed to copy the data into the HID buffer
	case -412: return "E_ERR_COPY_TX_DATA_FAILED";

		// The i2c engine (inside mcp2221) was already idle. The cancellation command had no effect.
	case -411: return "E_ERR_NO_EFFECT";
	case -413: return "E_ERR_INVALID_PEC";

		// The slave sent a different value for the block size(byte count) than we expected
	case -414: return "E_ERR_BLOCK_SIZE_MISMATCH";

	case -301: return "E_ERR_RAW_TX_TOO_LARGE";
	case -302: return "E_ERR_RAW_TX_COPYFAILED";
	case -303: return "E_ERR_RAW_RX_COPYFAILED";

	default: return "No Equivalent Error Code";
	}
}