################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
/home/raphael/code13offboard/shared/Networking/NetBuffer.cpp \
/home/raphael/code13offboard/shared/Networking/NetClient.cpp \
/home/raphael/code13offboard/shared/Networking/NetConnection.cpp \
/home/raphael/code13offboard/shared/Networking/NetPeer.cpp \
/home/raphael/code13offboard/shared/Networking/NetServer.cpp \
/home/raphael/code13offboard/shared/Networking/NetUtil.cpp 

OBJS += \
./Networking/NetBuffer.o \
./Networking/NetClient.o \
./Networking/NetConnection.o \
./Networking/NetPeer.o \
./Networking/NetServer.o \
./Networking/NetUtil.o 

CPP_DEPS += \
./Networking/NetBuffer.d \
./Networking/NetClient.d \
./Networking/NetConnection.d \
./Networking/NetPeer.d \
./Networking/NetServer.d \
./Networking/NetUtil.d 


# Each subdirectory must supply rules for building sources it contributes
Networking/NetBuffer.o: /home/raphael/code13offboard/shared/Networking/NetBuffer.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/usr/local/include/unicap -I/usr/include/libpng12 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/gtk-2.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/gtk-unix-print-2.0 -I/usr/include/gio-unix-2.0/ -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/atk-1.0 -I../../../code13offboard/shared/Networking -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Networking/NetClient.o: /home/raphael/code13offboard/shared/Networking/NetClient.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/usr/local/include/unicap -I/usr/include/libpng12 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/gtk-2.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/gtk-unix-print-2.0 -I/usr/include/gio-unix-2.0/ -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/atk-1.0 -I../../../code13offboard/shared/Networking -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Networking/NetConnection.o: /home/raphael/code13offboard/shared/Networking/NetConnection.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/usr/local/include/unicap -I/usr/include/libpng12 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/gtk-2.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/gtk-unix-print-2.0 -I/usr/include/gio-unix-2.0/ -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/atk-1.0 -I../../../code13offboard/shared/Networking -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Networking/NetPeer.o: /home/raphael/code13offboard/shared/Networking/NetPeer.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/usr/local/include/unicap -I/usr/include/libpng12 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/gtk-2.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/gtk-unix-print-2.0 -I/usr/include/gio-unix-2.0/ -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/atk-1.0 -I../../../code13offboard/shared/Networking -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Networking/NetServer.o: /home/raphael/code13offboard/shared/Networking/NetServer.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/usr/local/include/unicap -I/usr/include/libpng12 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/gtk-2.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/gtk-unix-print-2.0 -I/usr/include/gio-unix-2.0/ -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/atk-1.0 -I../../../code13offboard/shared/Networking -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Networking/NetUtil.o: /home/raphael/code13offboard/shared/Networking/NetUtil.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -I/usr/local/include/unicap -I/usr/include/libpng12 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/gtk-2.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/gtk-unix-print-2.0 -I/usr/include/gio-unix-2.0/ -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/atk-1.0 -I../../../code13offboard/shared/Networking -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


