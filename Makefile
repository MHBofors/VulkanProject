CFLAGS = -std=c17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

VulkanProject: main.c
	gcc $(CFLAGS) -o VulkanProject main.c $(LDFLAGS)

.PHONY: test clean

test: VulkanProject
	./VulkanProject

clean:
	rm -f VulkanProject