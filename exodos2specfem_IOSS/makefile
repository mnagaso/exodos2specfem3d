COMPILER  = g++
CFLAGS    = -std=c++11 -g -MMD -MP -Wall -Wextra -Winit-self -Wno-missing-field-initializers -DNO_XDMF_SUPPORT
ifeq "$(shell getconf LONG_BIT)" "64"
  LDFLAGS = -L./../seacas/lib -lio_info_lib -lIonit -lIotr -lIohb -lIogn -lIoexo_fac -lIofx -lIoex -lIoss -lexodus -lzoltan -lsimpi 
else
  LDFLAGS =
endif
LIBS      =
INCLUDE   = -I./include -I./../seacas/packages/seacas/libraries/ioss/src/main -I./../seacas/packages/seacas/libraries/ioss/src -I./../seacas/include
TARGET    = ./bin/$(shell basename `readlink -f .`)
SRCDIR    = ./src
ifeq "$(strip $(SRCDIR))" ""
  SRCDIR  = .
endif
SOURCES   = $(wildcard $(SRCDIR)/*.cpp)
OBJDIR    = ./obj
ifeq "$(strip $(OBJDIR))" ""
  OBJDIR  = .
endif
OBJECTS   = $(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.cpp=.o)))
DEPENDS   = $(OBJECTS:.o=.d)

$(TARGET): $(OBJECTS) $(LIBS)
	$(COMPILER) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	-mkdir -p $(OBJDIR)
	$(COMPILER) $(CFLAGS) $(INCLUDE) -o $@ -c $<

all: clean $(TARGET)

clean:
	-rm -f $(OBJECTS) $(DEPENDS) $(TARGET)

-include $(DEPENDS)
