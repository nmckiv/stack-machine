# ------------------------------------------------------------------
#  TOOLCHAIN
CXX      := g++
CXXFLAGS := -std=c++17 -I./ftxui -O2 #-Wall -Wextra
LDFLAGS  := -L./ftxui
LIBS     := -lftxui-component -lftxui-dom -lftxui-screen

# ------------------------------------------------------------------
#  SOURCE LISTS
APP_SRCS := main.cpp machine.cpp instructions.cpp tui.cpp console.cpp
LIB_SRCS := machine.cpp instructions.cpp globals.cpp          # only VM implementation
LIB_OBJS := $(LIB_SRCS:.cpp=.o)

# ------------------------------------------------------------------
#  ORIGINAL “MONOLITHIC” EXECUTABLE  (unchanged behaviour)
TARGET := stack-machine

$(TARGET): $(APP_SRCS)
	@echo "  LINK  $@  (monolithic)"
	$(CXX) $(APP_SRCS) -o $@ $(CXXFLAGS) $(LDFLAGS) $(LIBS)

# ------------------------------------------------------------------
#  STATIC LIBRARY
LIBNAME := libstackvm.a

$(LIBNAME): $(LIB_OBJS)
	@echo "  AR    $@"
	@ar rcs $@ $^

%.o: %.cpp
	@echo "  CXX   $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# convenience aliases
.PHONY: lib
lib: $(LIBNAME)

.PHONY: clean-lib
clean-lib:
	@rm -f $(LIB_OBJS) $(LIBNAME)
	@echo "  CLEAN libstackvm.a and object files"

# ------------------------------------------------------------------
#  DRIVER BUILT FROM GENERATED CODE  (uses the static library)
#  Assumes autogen.cpp is produced by your compiler.
MYPROG := myprog
AUTOGEN := autogen.cpp           # change if your generator writes a different name

$(MYPROG): $(AUTOGEN) $(LIBNAME)
	@echo "  LINK  $@  (driver)"
	$(CXX) $(CXXFLAGS) -I. $< -o $@ -L. -lstackvm $(LDFLAGS) $(LIBS)

# ------------------------------------------------------------------
#  GLOBAL CLEAN
.PHONY: clean
clean: clean-lib
	@rm -f $(TARGET) $(MYPROG)
	@echo "  CLEAN executable(s)"
