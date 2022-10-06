#pragma once

/**
 * @file bootrom.h
 * @brief Game Boy Boot ROM
 *
 * @author J.-C. Chappelier
 * @date 11/2019
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "bus.h"
#include "component.h"
#include "gameboy.h"

/**
 * @brief Game Boy Boot ROM content.
 */

#define GAMEBOY_BOOT_ROM_CONTENT { 					\
	0x31, 0xFE, 0xFF, 0x21, 0x00, 0x80, 0x22, 0xCB, \
    0x6C, 0x28, 0xFB, 0x3E, 0x80, 0xE0, 0x26, 0xE0, \
    0x11, 0x3E, 0xF3, 0xE0, 0x12, 0xE0, 0x25, 0x3E, \
    0x77, 0xE0, 0x24, 0x3E, 0xFC, 0xE0, 0x47, 0x11, \
    0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0x47, 0xCD, \
    0x82, 0x00, 0xCD, 0x82, 0x00, 0x13, 0x7B, 0xEE, \
    0x34, 0x20, 0xF2, 0x11, 0xB1, 0x00, 0x0E, 0x08, \
    0x1A, 0x13, 0x22, 0x23, 0x0D, 0x20, 0xF9, 0x3E, \
    0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, \
    0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20, 0xF9, \
    0x2E, 0x0F, 0x18, 0xF5, 0x3E, 0x91, 0xE0, 0x40, \
    0x06, 0x2D, 0xCD, 0xA3, 0x00, 0x3E, 0x83, 0xCD, \
    0xAA, 0x00, 0x06, 0x05, 0xCD, 0xA3, 0x00, 0x3E, \
    0xC1, 0xCD, 0xAA, 0x00, 0x06, 0x46, 0xCD, 0xA3, \
    0x00, 0x21, 0xB0, 0x01, 0xE5, 0xF1, 0x21, 0x4D, \
    0x01, 0x01, 0x13, 0x00, 0x11, 0xD8, 0x00, 0xC3, \
    0xFE, 0x00, 0x3E, 0x04, 0x0E, 0x00, 0xCB, 0x20, \
    0xF5, 0xCB, 0x11, 0xF1, 0xCB, 0x11, 0x3D, 0x20, \
    0xF5, 0x79, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xE5, \
    0x21, 0x0F, 0xFF, 0xCB, 0x86, 0xCB, 0x46, 0x28, \
    0xFC, 0xE1, 0xC9, 0xCD, 0x97, 0x00, 0x05, 0x20, \
    0xFA, 0xC9, 0xE0, 0x13, 0x3E, 0x87, 0xE0, 0x14, \
    0xC9, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, \
    0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x50  \
}


/**
 * @brief Writes bootrom content to a component
 *
 * @param c component to write the bootrom content to
 * @return error code
 */
int bootrom_init(component_t* c);


/**
 * @brief Macro to plug bootrom onto the bus
 */
#define bootrom_plug(c, bus) bus_forced_plug(bus, c, BOOT_ROM_START, BOOT_ROM_END, 0)


/**
 * @brief Bootrom bus listening handler
 *
 * @param gameboy gameboy
 * @param address trigger address
 * @return error code
 */
int bootrom_bus_listener(gameboy_t* gameboy, addr_t addr);

#ifdef __cplusplus
}
#endif
