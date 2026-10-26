CXX      := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -O2 -I. -MMD -MP
LDLIBS   := -lsfml-graphics -lsfml-window -lsfml-system -lstdc++exp

TARGET := pins.exe
SRCS   := pins.cpp
OBJS   := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(OBJS:.o=.d) $(TARGET)

-include $(OBJS:.o=.d)

.PHONY: all clean