#常用定义选项
ZMOD_NAME		= easyice
ZMOD_VERSION		= 1.0.3
MACHINE		= $(shell uname -m)
OUTPUT_INCLUDE_PATH	=  ../../sdk/include/
OUTPUT_LIB_PATH		= ../../sdk/lib/

SRC_PATH		= ../src/
SRC_FILES		= $(wildcard ../../common/TsPacket.cpp \
							${SRC_PATH}/libeasyice.cpp \
							../../common/jmdec.cpp \
							../../common/utils.cpp \
							../../common/zevent.cpp \
							../../deps/jsoncpp-0.10.6/src/lib_json/json_value.cpp \
							../../deps/jsoncpp-0.10.6/src/lib_json/json_reader.cpp\
							../../deps/jsoncpp-0.10.6/src/lib_json/json_writer.cpp \
							${SRC_PATH}/EasyICEDLL/TrView.cpp \
							${SRC_PATH}/EasyICEDLL/TrMsgView.cpp \
							${SRC_PATH}/EasyICEDLL/TrMsgMgr.cpp \
							${SRC_PATH}/EasyICEDLL/FileAnalysis.cpp \
							${SRC_PATH}/EasyICEDLL/EiLog.cpp \
							${SRC_PATH}/EasyICEDLL/MpegDec.cpp \
							${SRC_PATH}/EasyICEDLL/DemuxTs.cpp \
							${SRC_PATH}/EasyICEDLL//ProgramParser.cpp \
							${SRC_PATH}/EasyICEDLL/DetectStreamType.cpp \
							${SRC_PATH}/EasyICEDLL/CheckMediaInfo.cpp \
							${SRC_PATH}/EasyICEDLL/PcrOj.cpp \
							${SRC_PATH}/EasyICEDLL/CircularBuffer.cpp\
							${SRC_PATH}/EasyICEDLL/StreamFilterBase.cpp \
							${SRC_PATH}/EasyICEDLL/LiveSourceBase.cpp \
							${SRC_PATH}/EasyICEDLL/StreamFilterRtp.cpp \
							${SRC_PATH}/EasyICEDLL/LiveAnalysis.cpp \
							${SRC_PATH}/EasyICEDLL/LiveSourceFactory.cpp \
							${SRC_PATH}/EasyICEDLL/LiveAnalysisImpl.cpp \
							${SRC_PATH}/EasyICEDLL/LiveSourceUdp.cpp \
							${SRC_PATH}/EasyICEDLL/LivePcrProc.cpp \
							${SRC_PATH}/EasyICEDLL/ccalcpcrn1.cpp \
							${SRC_PATH}/EasyICEDLL/cudpobj.cpp \
							${SRC_PATH}/EasyICEDLL/tables/*.cpp \
							${SRC_PATH}/EasyICEDLL/tables/section/*.cpp \
							${SRC_PATH}/H264DecDll/JM17.2/ldecod/src/cabac.c\
							${SRC_PATH}/H264DecDll/JM17.2/ldecod/src/header.c\
							${SRC_PATH}/H264DecDll/JM17.2/ldecod/src/image.c\
							${SRC_PATH}/H264DecDll/JM17.2/ldecod/src/ldecod.c\
							${SRC_PATH}/H264DecDll/JM17.2/ldecod/src/mbuffer.c\
							${SRC_PATH}/H264DecDll/JM17.2/ldecod/src/mbuffer_mvc.c\
							${SRC_PATH}/H264DecDll/JM17.2/ldecod/src/mc_prediction.c\
							${SRC_PATH}/H264DecDll/JM17.2/lcommon/src/memalloc.c\
							${SRC_PATH}/H264DecDll/JM17.2/ldecod/src/nal.c\
							${SRC_PATH}/H264DecDll/JM17.2/ldecod/src/parset.c\
							${SRC_PATH}/H264DecDll/JM17.2/lcommon/src/parsetcommon.c\
							${SRC_PATH}/H264DecDll/JM17.2/ldecod/src/vlc.c\
							${SRC_PATH}/H264DecDll/H264DecodeCore.cpp\
							${SRC_PATH}/H264DecDll/H264Dec.cpp)


# 将C 接口不需要的tablesdefs.h commondefs.h一同拷贝，避免文件不一致，并留做 C++接口备用
EXPORT_INCLUDE_FILES = libeasyice.h  sdkdefs.h tablesdefs.h commondefs.h EasyICEDLL/EiLog.h EasyICEDLL/string_res.h

USING_LIBS		= -lmediainfo -ldl -lhlsanalysis
USING_INCLUDES_PATH	= -I../src/H264DecDll/JM17.2/lcommon/inc \
					-I../src/H264DecDll/JM17.2/ldecod/inc \
                	-I../src/ \
                	-I../src/H264DecDll/ \
                	-I../../common/ \
                	-I../../sdk/include \
                	-I../src/EasyICEDLL
USING_LIBS_PATH		= -L../../sdk/lib/

DEBUG  			= YES
DEFINES			=



#通用定义
GCC			= gcc
G++			= g++
LD 			= g++

ZMOD_BUILD_DATE         = $(shell date)

DEBUG_CFLAGS     := -c -Wall -Wno-unused -shared -fPIC -Wno-format -g -O2 -DDEBUG -fexceptions -fnon-call-exceptions
RELEASE_CFLAGS   := -c -Wall -Wno-unused -shared -fPIC -Wno-unknown-pragmas -Wno-format -O3 -fexceptions -fnon-call-exceptions 

DEBUG_LDFLAGS    := -g -shared -fPIC -Wl,-soname,lib$(ZMOD_NAME).so
RELEASE_LDFLAGS  := -shared -fPIC -Wl,-soname,lib$(ZMOD_NAME).so


ifeq (YES, ${DEBUG})
	CFLAGS       	:= ${DEBUG_CFLAGS}
	LDFLAGS      	:= ${DEBUG_LDFLAGS}
	USING_LIBS_PATH	:= $(USING_LIBS_PATH)
else
	CFLAGS      	:= ${RELEASE_CFLAGS}
	LDFLAGS      	:= ${RELEASE_LDFLAGS}
	USING_LIBS_PATH	:= $(addsuffix release/,$(USING_LIBS_PATH))
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
OUTPUT 		= $(OUTPUT_LIB_PATH)$(PROGRAM_NAME)

	ZMOD_BUILD_DATE		= $(shell date)

OUTPUT 			= $(OUTPUT_LIB_PATH)lib$(ZMOD_NAME).so.$(ZMOD_VERSION)
CFLAGS		+=  -DZMOD_BUILD_DATE="\"$(ZMOD_BUILD_DATE)\"" -DZMOD_VERSION="\"$(ZMOD_VERSION)\""

VPATH		= $(SRC_PATH)
EXPORT_INCLUDE_FILES := $(addprefix $(SRC_PATH),$(EXPORT_INCLUDE_FILES))

default:$(OUTPUT)

	@ln -sf ./lib$(ZMOD_NAME).so.$(ZMOD_VERSION) $(OUTPUT_LIB_PATH)lib$(ZMOD_NAME).so
	@cp $(EXPORT_INCLUDE_FILES) $(OUTPUT_INCLUDE_PATH)
	@echo "$(OUTPUT) MAKE 处理完毕"

$(OUTPUT):$(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(USING_LIBS_PATH) $(USING_LIBS)
	@echo "\\33[35m\\33[1m[链接]\\33[21m $@ 执行完毕，总计更新处理：$(words $?) 个文件\\33[0m"

%.o:%.c
	@$(call make-depend,$<,$@,$(subst .o,.d,$@))
	$(GCC) $(CFLAGS) $< -o $@
	@echo "\\33[32m\\33[1m[编译]\\33[21m $< 执行完毕\\33[0m"

%.o:%.cpp
	@$(call make-depend,$<,$@,$(subst .o,.d,$@))
	@$(G++) $(CFLAGS) $< -o $@
	@echo "\\33[32m\\33[1m[编译]\\33[21m $< 执行完毕\\33[0m"

-include $(DEPENDS)

help:
	@echo "\\33[1m\\33[33m============== 通用可执行文件MAKEFILE ==============\\33[0m"
	@echo "\\33[33m  2007.2 Writen By ZhuFeng \\33[0m"
	@echo "\\33[32m\\33[1mmake		:\\33[21m 缺省调用方式，使用debug模式进行编译与连接，输出二进制文件到指定目录\\33[0m"
	@echo "\\33[32m\\33[1mmake DEBUG=NO	:\\33[21m 使用release模式进行编译与连接，输出二进制文件到指定目录 \\33[0m"
	@echo "\\33[32m\\33[1mmake help	:\\33[21m显示帮助信息\\33[0m"
	@echo "\\33[32m\\33[1mmake rebuild	:\\33[21m重新生成所有文件\\33[0m"
	@echo "\\33[32m\\33[1mmake clean	:\\33[21m清除生成的中间文件\\33[0m"


rebuild:clean default
	@echo "\\33[34m\\33[1m[rebuild]\\33[21m 执行完毕\\33[0m"
clean:
	@rm -f *.o
	@rm -f *.d
	@find $(SRC_PATH) -name "*.o" |xargs rm -f
	@find $(SRC_PATH) -name "*.d" |xargs rm -f
	@find ../../common/  -name "*.o" |xargs rm -f
	@find ../../common/  -name "*.d" |xargs rm -f
	@find ../../deps/jsoncpp-0.10.6/src/lib_json/  -name "*.o" |xargs rm -f
	@find ../../deps/jsoncpp-0.10.6/src/lib_json/  -name "*.d" |xargs rm -f
	@rm -f $(OUTPUT)
	@echo "\\33[34m\\33[1m[clean]\\33[21m 执行完毕\\33[0m"

