#pragma once
#include <cstdint>

namespace ad7193 {

	enum class Reg : uint8_t {
	  STATUS = 0,
	  MODE = 1,
	  CONFIG = 2,
	  DATA = 3,
	  ID = 4,
	  GPOCON = 5,
	  OFFSET = 6,
	  FULLSCALE = 7
	};

	// Status Register (8-bit)
	namespace status {
		constexpr uint8_t RDY    = (1u << 7); // active low -> 1 = not ready, 0 = ready
		constexpr uint8_t ERR    = (1u << 6);
		constexpr uint8_t NOREF  = (1u << 5);
		constexpr uint8_t PARITY = (1u << 4);
		constexpr uint8_t CH_MASK = 0x0F;

	  	inline bool ready(uint8_t sr) { return (sr & RDY) == 0; }
	  	inline uint8_t channel_id(uint8_t sr) { return sr & CH_MASK; }
	}
	
	// Mode Register (24-bit)
	namespace mode {
	  	constexpr uint32_t CONTINUOUS        = (0u << 21);
	  	constexpr uint32_t SINGLE_CONV       = (1u << 21);
	  	constexpr uint32_t IDLE              = (2u << 21);
	  	constexpr uint32_t PWR_DWN           = (3u << 21);
	  	constexpr uint32_t ZERO_SCALE_INT    = (4u << 21);
	  	constexpr uint32_t FULL_SCALE_INT    = (5u << 21);
	  	constexpr uint32_t ZERO_SCALE_SYS    = (6u << 21);
	  	constexpr uint32_t FULL_SCALE_SYS    = (7u << 21);
	  	constexpr uint32_t DAT_STA 	     = (1u << 20);
	
	  	// FS field helper (MR9..MR0)
	  	inline constexpr uint32_t FS(uint16_t fs) { return uint32_t(fs & 0x3FFu); }
	}

	// Config Register (24 bit)
	namespace config {
		constexpr uint32_t 
} 

