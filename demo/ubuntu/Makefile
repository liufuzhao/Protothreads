all_obj = pt_demo
all:$(all_obj)
PT_CORE_FILE_PATH = ../../protothreads/*.c ../../protothreads_test/*.c ./*.c
PT_CORE_OBJ = $(patsubst %.c,%.o,$(wildcard $(PT_CORE_FILE_PATH)))
INCLUDE_PATH = -I ../../protothreads -I ../../protothreads_test  -I ./

PT_CORE_I = $(patsubst %.c,%.i,$(wildcard $(PT_CORE_FILE_PATH)))

%.o: %.c
	$(CC) $(CFLAGS)  $(INCLUDE_PATH) -fdiagnostics-color=always -g -c -o $@ $<

pt_demo: $(PT_CORE_OBJ)
	$(CC) $(CFLAGS) -g -o $@ $(PT_CORE_OBJ) $(LDFLAGS) -lpthread 

clean:
	rm -f *.o $(PT_CORE_OBJ) $(PT_CORE_OBJ) $(PT_CORE_I)

%.i: %.c
	$(CC) $(CFLAGS) -E  $(INCLUDE_PATH) $< -o $@


READ: $(PT_CORE_I)
