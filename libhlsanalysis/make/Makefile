#****************************************************************************
# 通用的生成动态库的 c/c++ GNU Makefile 
# Writen By ZhangBaoZhu,2007.7
# 功能：自动从所有的.c或.cpp文件列表中生成输出文件，可以生成动态库
#****************************************************************************

#常用定义选项
LIB_NAME		= hlsanalysis
VERSION			= 1.0.0
OUTPUT_BIN_PATH		= ../../sdk/lib/
OUTPUT_INCLUDE_PATH	= ../../sdk/include/
SRC_PATH		= ../src/

SRC_FILES               = $(wildcard ../../common/TsPacket.cpp \
				../../common/CBit.cpp \
				../../common/jmdec.cpp \
				../../common/zevent.cpp \
				${SRC_PATH}/*.cpp \
				${SRC_PATH}/PsiCheck.cpp)
				

EXPORT_INCLUDE_FILES	= HlsAnalysis.h hls_define.h

USING_LIBS		= 
USING_INCLUDES_PATH	= -I$(OUTPUT_INCLUDE_PATH) \
				-I../../common/ 
USING_LIBS_PATH		= -L$(OUTPUT_BIN_PATH)

DEBUG  		= YES
DEFINES			= 



#通用定义
GCC			= g++
LD 			= g++

DEBUG_CFLAGS     := -c -Wall -shared -fPIC -Wno-format -g -DDEBUG
RELEASE_CFLAGS   := -c -Wall -shared -fPIC -Wno-unknown-pragmas -Wno-format -O3

DEBUG_LDFLAGS    := -g  -shared -fPIC -Wl,-soname,lib$(LIB_NAME).so
RELEASE_LDFLAGS  := -shared -fPIC -Wl,-soname,lib$(LIB_NAME).so


ifeq (YES, ${DEBUG})
	CFLAGS       	:= ${DEBUG_CFLAGS}
	LDFLAGS      	:= ${DEBUG_LDFLAGS}
	USING_LIBS_PATH	:= $(USING_LIBS_PATH)
	OUTPUT_PATH	:= $(OUTPUT_BIN_PATH)/
else
	CFLAGS      	:= ${RELEASE_CFLAGS}
	LDFLAGS      	:= ${RELEASE_LDFLAGS}
	USING_LIBS_PATH	:= $(addsuffix release/,$(USING_LIBS_PATH))
	OUTPUT_PATH	:= $(OUTPUT_BIN_PATH)release/
endif

#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS		+= $(DEFINES)
CFLAGS		+= $(USING_INCLUDES_PATH)

#****************************************************************************
# 定义自动化编译宏，使用GCC方式
#****************************************************************************
# $(call make-depend,source-file,object-file,depend-file)
define make-depend
	$(GCC) -MM -MF $3 -MP -MT $2 $(CFLAGS) $1
endef

OBJS 		= $(addsuffix .o,$(basename $(SRC_FILES)))
DEPENDS 	= $(addsuffix .d,$(basename $(SRC_FILES)))
OUTPUT 		= $(OUTPUT_PATH)lib$(LIB_NAME).so.$(VERSION)

VPATH		= $(SRC_PATH)
EXPORT_INCLUDE_FILES := $(addprefix $(SRC_PATH),$(EXPORT_INCLUDE_FILES))

default:$(OUTPUT)
	@ln -sf ./lib$(LIB_NAME).so.$(VERSION) $(OUTPUT_PATH)lib$(LIB_NAME).so
	@cp $(EXPORT_INCLUDE_FILES) $(OUTPUT_INCLUDE_PATH)
	@echo "$(OUTPUT) MAKE 处理完毕"
	@echo "\\33[32m\\33[1m[生成]\\33[21m 执行完毕!\\12$(OUTPUT) 被生成 \\12$(EXPORT_INCLUDE_FILES)被复制到$(OUTPUT_INCLUDE_PATH)\\33[0m"
	

$(OUTPUT):$(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(USING_LIBS_PATH) $(USING_LIBS)
	@echo "\\33[35m\\33[1m[链接]\\33[21m $@ 执行完毕，总计更新处理：$(words $?) 个文件\\33[0m"

%.o:%.c
	@$(call make-depend,$<,$@,$(subst .o,.d,$@))
	$(GCC) $(CFLAGS) $< -o $@
	@echo "\\33[32m\\33[1m[编译]\\33[21m $< 执行完毕\\33[0m"
	
%.o:%.cpp
	@$(call make-depend,$<,$@,$(subst .o,.d,$@))
	@$(GCC) $(CFLAGS) $< -o $@
	@echo "\\33[32m\\33[1m[编译]\\33[21m $< 执行完毕\\33[0m"

-include $(DEPENDS)

help:
	@echo "\\33[1m\\33[33m============== 通用动态库MAKEFILE ==============\\33[0m"
	@echo "\\33[33m  2007.2 Writen By ZhuFeng \\33[0m"
	@echo "\\33[32m\\33[1mmake		:\\33[21m 缺省调用方式，使用debug模式进行编译与连接，输出二进制文件到指定目录\\33[0m"
	@echo "\\33[32m\\33[1mmake DEBUG=NO	:\\33[21m 使用release模式进行编译与连接，输出二进制文件到指定目录 \\33[0m"
	@echo "\\33[32m\\33[1mmake help	:\\33[21m显示帮助信息\\33[0m"
	@echo "\\33[32m\\33[1mmake rebuild	:\\33[21m重新生成所有文件\\33[0m"
	@echo "\\33[32m\\33[1mmake clean	:\\33[21m清除生成的中间文件\\33[0m"
#	@echo "\\33[32m\\33[1mmake install	:\\33[21m安装生成的文件到/usr/local/lib目录，拷贝头文件到/usr/local/include目录\\33[0m"


rebuild:clean default
	@echo "\\33[34m\\33[1m[rebuild]\\33[21m 执行完毕\\33[0m"



clean:
	@rm -f *.o
	@rm -f *.d
	@find ../../common/ -name "*.o" |xargs rm -f
	@find ../../common/ -name "*.d" |xargs rm -f
	@find $(SRC_PATH) -name "*.o" |xargs rm -f
	@find $(SRC_PATH) -name "*.d" |xargs rm -f
	@rm -f $(OUTPUT)
	@echo "\\33[34m\\33[1m[clean]\\33[21m 执行完毕\\33[0m"




#install:
#	@sudo cp $(OUTPUT) /usr/lib/
#	@sudo ln -sf /usr/lib/$(notdir $(OUTPUT)) /usr/lib/lib$(LIB_NAME).so
#	@sudo cp $(EXPORT_INCLUDE_FILES) /usr/include/
#	@echo "\\33[32m\\33[1m[安装]\\33[21m 执行完毕!\\12$(OUTPUT) 被安装到/user/lib \\12$(EXPORT_INCLUDE_FILES)被安装到/usr/include\\33[0m"
	
