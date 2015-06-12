APP_OBJ = main.o  dbg.o

LCD_OBJ = ./C12832/TextDisplay.o ./C12832/GraphicsDisplay.o ./C12832/C12832.o
LCD_INC = -I./C12832

ACCELEROMETERS_OBJ = ./MMA7660/MMA7660.o
ACCELEROMETERS_INC = -I./MMA7660

TEMPERATURE_OBJ = ./LM75B/LM75B.o
TEMPERATURE_INC = -I./LM75B

WAKAAMA_CLIENT_OBJ = ./wakaama/client_objects/object_device.o ./wakaama/client_objects/object_security.o ./wakaama/client_objects/object_firmware.o ./wakaama/client_objects/object_server.o
WAKAAMA_OBJ = $(WAKAAMA_CLIENT_OBJ) ./wakaama/observe.o ./wakaama/transaction.o ./wakaama/bootstrap.o ./wakaama/list.o ./wakaama/liblwm2m.o ./wakaama/utils.o ./wakaama/objects.o ./wakaama/packet.o ./wakaama/tlv.o ./wakaama/management.o ./wakaama/uri.o ./wakaama/registration.o ./wakaama/er-coap-13/er-coap-13.o
WAKAAMA_INC = -I./wakaama -I./wakaama/er-coap-13
WAKAAMA_SYM = -DLWM2M_LITTLE_ENDIAN -DLWM2M_CLIENT_MODE
WAKAAMA_SYM_DEBUG = -DWITH_LOGS

ETHERNET_OBJ = ./EthernetInterface/lwip/core/mem.o ./EthernetInterface/lwip/core/tcp.o ./EthernetInterface/lwip/core/netif.o ./EthernetInterface/lwip/core/tcp_in.o ./EthernetInterface/lwip/core/dhcp.o ./EthernetInterface/lwip/core/memp.o ./EthernetInterface/lwip/core/tcp_out.o ./EthernetInterface/lwip/core/udp.o ./EthernetInterface/lwip/core/def.o ./EthernetInterface/lwip/core/stats.o ./EthernetInterface/lwip/core/dns.o ./EthernetInterface/lwip/core/raw.o ./EthernetInterface/lwip/core/timers.o ./EthernetInterface/lwip/core/pbuf.o ./EthernetInterface/lwip/core/init.o ./EthernetInterface/lwip/core/ipv4/igmp.o ./EthernetInterface/lwip/core/ipv4/ip_frag.o ./EthernetInterface/lwip/core/ipv4/autoip.o ./EthernetInterface/lwip/core/ipv4/inet.o ./EthernetInterface/lwip/core/ipv4/icmp.o ./EthernetInterface/lwip/core/ipv4/ip_addr.o ./EthernetInterface/lwip/core/ipv4/inet_chksum.o ./EthernetInterface/lwip/core/ipv4/ip.o ./EthernetInterface/lwip/core/snmp/msg_in.o ./EthernetInterface/lwip/core/snmp/asn1_enc.o ./EthernetInterface/lwip/core/snmp/mib_structs.o ./EthernetInterface/lwip/core/snmp/asn1_dec.o ./EthernetInterface/lwip/core/snmp/mib2.o ./EthernetInterface/lwip/core/snmp/msg_out.o ./EthernetInterface/lwip/api/netifapi.o ./EthernetInterface/lwip/api/sockets.o ./EthernetInterface/lwip/api/netbuf.o ./EthernetInterface/lwip/api/netdb.o ./EthernetInterface/lwip/api/api_lib.o ./EthernetInterface/lwip/api/err.o ./EthernetInterface/lwip/api/api_msg.o ./EthernetInterface/lwip/api/tcpip.o ./EthernetInterface/lwip/netif/etharp.o ./EthernetInterface/lwip/netif/slipif.o ./EthernetInterface/lwip/netif/ethernetif.o ./EthernetInterface/lwip/netif/ppp/auth.o ./EthernetInterface/lwip/netif/ppp/pap.o ./EthernetInterface/lwip/netif/ppp/randm.o ./EthernetInterface/lwip/netif/ppp/md5.o ./EthernetInterface/lwip/netif/ppp/lcp.o ./EthernetInterface/lwip/netif/ppp/magic.o ./EthernetInterface/lwip/netif/ppp/chap.o ./EthernetInterface/lwip/netif/ppp/ppp.o ./EthernetInterface/lwip/netif/ppp/ppp_oe.o ./EthernetInterface/lwip/netif/ppp/ipcp.o ./EthernetInterface/lwip/netif/ppp/vj.o ./EthernetInterface/lwip/netif/ppp/fsm.o ./EthernetInterface/lwip/netif/ppp/chpms.o ./EthernetInterface/lwip-sys/arch/sys_arch.o ./EthernetInterface/lwip-sys/arch/checksum.o ./EthernetInterface/lwip-sys/arch/memcpy.o ./EthernetInterface/lwip-eth/arch/TARGET_NXP/lpc17_emac.o ./EthernetInterface/lwip-eth/arch/TARGET_NXP/lpc_phy_dp83848.o ./EthernetInterface/EthernetInterface.o ./EthernetInterface/Socket/Endpoint.o ./EthernetInterface/Socket/TCPSocketServer.o ./EthernetInterface/Socket/UDPSocket.o ./EthernetInterface/Socket/Socket.o ./EthernetInterface/Socket/TCPSocketConnection.o
ETHERNET_INC = -I./EthernetInterface -I./EthernetInterface/Socket -I./EthernetInterface/lwip -I./EthernetInterface/lwip/core -I./EthernetInterface/lwip/core/ipv4 -I./EthernetInterface/lwip/core/snmp -I./EthernetInterface/lwip/api -I./EthernetInterface/lwip/netif -I./EthernetInterface/lwip/netif/ppp -I./EthernetInterface/lwip/include -I./EthernetInterface/lwip/include/ipv4 -I./EthernetInterface/lwip/include/ipv4/lwip -I./EthernetInterface/lwip/include/lwip -I./EthernetInterface/lwip/include/netif -I./EthernetInterface/lwip-sys -I./EthernetInterface/lwip-sys/arch -I./EthernetInterface/lwip-eth -I./EthernetInterface/lwip-eth/arch -I./EthernetInterface/lwip-eth/arch/TARGET_NXP

RTOS_OBJ = ./mbed-rtos/rtx/TARGET_CORTEX_M/TARGET_M3/TOOLCHAIN_GCC/SVC_Table.o ./mbed-rtos/rtx/TARGET_CORTEX_M/TARGET_M3/TOOLCHAIN_GCC/HAL_CM3.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_Semaphore.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_Event.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_List.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_Mutex.o ./mbed-rtos/rtx/TARGET_CORTEX_M/HAL_CM.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_Task.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_CMSIS.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_System.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_Time.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_MemBox.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_Robin.o ./mbed-rtos/rtx/TARGET_CORTEX_M/RTX_Conf_CM.o ./mbed-rtos/rtx/TARGET_CORTEX_M/rt_Mailbox.o ./mbed-rtos/rtos/Thread.o ./mbed-rtos/rtos/Semaphore.o ./mbed-rtos/rtos/Mutex.o ./mbed-rtos/rtos/RtosTimer.o 
RTOS_INC = -I./mbed-rtos -I./mbed-rtos/rtos -I./mbed-rtos/rtx -I./mbed-rtos/rtx/TARGET_CORTEX_M -I./mbed-rtos/rtx/TARGET_CORTEX_M/TARGET_M3 -I./mbed-rtos/rtx/TARGET_CORTEX_M/TARGET_M3/TOOLCHAIN_GCC
###############################################################################
GCC_BIN = 
PROJECT = lpc1768_lwm2m
OBJECTS = $(APP_OBJ) $(LCD_OBJ) $(TEMPERATURE_OBJ) $(ACCELEROMETERS_OBJ) $(WAKAAMA_OBJ) $(ETHERNET_OBJ) $(RTOS_OBJ) 
SYS_OBJECTS = ./mbed/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/retarget.o ./mbed/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/system_LPC17xx.o ./mbed/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/board.o ./mbed/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/cmsis_nvic.o ./mbed/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/startup_LPC17xx.o 
INCLUDE_PATHS = $(LCD_INC) $(TEMPERATURE_INC) $(ACCELEROMETERS_INC) $(WAKAAMA_INC) $(ETHERNET_INC) $(RTOS_INC) -I. -I./mbed -I./mbed/TARGET_LPC1768 -I./mbed/TARGET_LPC1768/TOOLCHAIN_GCC_ARM -I./mbed/TARGET_LPC1768/TARGET_NXP -I./mbed/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X -I./mbed/TARGET_LPC1768/TARGET_NXP/TARGET_LPC176X/TARGET_MBED_LPC1768 
LIBRARY_PATHS = -L./mbed/TARGET_LPC1768/TOOLCHAIN_GCC_ARM 
LIBRARIES = -lmbed 
LINKER_SCRIPT = ./mbed/TARGET_LPC1768/TOOLCHAIN_GCC_ARM/LPC1768.ld

############################################################################### 
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy
OBJDUMP = $(GCC_BIN)arm-none-eabi-objdump
SIZE 	= $(GCC_BIN)arm-none-eabi-size

CPU = -mcpu=cortex-m3 -mthumb
CC_FLAGS = $(CPU) -c -g -fno-common -fmessage-length=0 -Wall -fno-exceptions -ffunction-sections -fdata-sections -fomit-frame-pointer
CC_FLAGS += -MMD -MP
CC_SYMBOLS = $(WAKAAMA_SYM) -DTARGET_LPC1768 -DTARGET_M3 -DTARGET_CORTEX_M -DTARGET_NXP -DTARGET_LPC176X -DTARGET_MBED_LPC1768 -DTOOLCHAIN_GCC_ARM -DTOOLCHAIN_GCC -D__CORTEX_M3 -DARM_MATH_CM3 -DMBED_BUILD_TIMESTAMP=1433768453.91 -D__MBED__=1 

LD_FLAGS = $(CPU) -Wl,--gc-sections --specs=nano.specs -u _printf_float -u _scanf_float -Wl,--wrap,main
LD_FLAGS += -Wl,-Map=$(PROJECT).map,--cref
LD_SYS_LIBS = -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys

ifeq ($(DEBUG), 1)
  CC_FLAGS += -DDEBUG -O0
  CC_SYMBOLS += -D__DEBUG__=4 # DEBUG level
  CC_SYMBOLS += ${WAKAAMA_SYM_DEBUG}
else
  CC_SYMBOLS += -D__DEBUG__=1 # ERROR level
  CC_FLAGS += -DNDEBUG -Os
endif

all: $(PROJECT).bin $(PROJECT).hex 

clean:
	rm -f $(PROJECT).bin $(PROJECT).elf $(PROJECT).hex $(PROJECT).map $(PROJECT).lst $(OBJECTS) $(DEPS)

.s.o:
	$(AS) $(CPU) -o $@ $<

.c.o:
	$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99   $(INCLUDE_PATHS) -o $@ $<

.cpp.o:
	$(CPP) $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu++98 -fno-rtti $(INCLUDE_PATHS) -o $@ $<


$(PROJECT).elf: $(OBJECTS) $(SYS_OBJECTS)
	$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ $^ $(LIBRARIES) $(LD_SYS_LIBS) $(LIBRARIES) $(LD_SYS_LIBS)
	@echo ""
	@echo "*****"
	@echo "***** You must modify vector checksum value in *.bin and *.hex files."
	@echo "*****"
	@echo ""
	$(SIZE) $@

$(PROJECT).bin: $(PROJECT).elf
	@$(OBJCOPY) -O binary $< $@

$(PROJECT).hex: $(PROJECT).elf
	@$(OBJCOPY) -O ihex $< $@

$(PROJECT).lst: $(PROJECT).elf
	@$(OBJDUMP) -Sdh $< > $@

lst: $(PROJECT).lst

size:
	$(SIZE) $(PROJECT).elf

DEPS = $(OBJECTS:.o=.d) $(SYS_OBJECTS:.o=.d)
-include $(DEPS)
