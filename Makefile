CXX = g++
CXXFLAGS = -std=c++2b -Wall -g -pthread

# Lista de archivos fuente
SRCS = main.cpp \
       containers/BinaryTreeDemo.cpp \
       containers/BinaryTreeAVLDemo.cpp \
       containers/BinaryTreeRBDemo.cpp

# Archivos objeto generados a partir de los fuentes
OBJS = $(SRCS:.cpp=.o)

# Nombre del ejecutable
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) temp.txt
