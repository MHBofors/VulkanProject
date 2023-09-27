CFLAGS = -std=c17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
DEPS = utils.h vkMath.h
OBJ = main.o utils.o vkMath.o

%.o: %.c $(DEPS)
	gcc $(CFLAGS) -c -o $@ $< $(LDFLAGS)

VulkanProject: $(OBJ)
	gcc $(CFLAGS) -o VulkanProject $(OBJ) $(LDFLAGS)
	rm -f $(OBJ)



.PHONY: test clean

test: VulkanProject
	./VulkanProject

clean:
	rm -f VulkanProject $(OBJ)