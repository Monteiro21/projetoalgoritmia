# use @ to silence

CC = gcc

CFLAGS = -Wall -Wextra -pedantic -O1 -fsanitize=address -fno-omit-frame-pointer -g
#remove -L system search path, and -I system header search path later
LDFLAGS = -I/opt/homebrew/include -L/opt/homebrew/lib -lcunit

SRCS = jogo.c
OBJS = $(SRCS:.c=.o)
SRCS_ALL = jogo.c cunit_tests.c
OBJS_ALL = $(SRCS_ALL:.c=.o)
TEST_FLAGS := -DRUN_TESTS -fprofile-arcs -ftest-coverage

JOGO_TARGET = jogo
TESTAR_TARGET = testar

ifeq ($(MAKECMDGOALS), $(TESTAR_TARGET))
	CFLAGS += $(TEST_FLAGS)
endif


all: $(JOGO_TARGET)

#(target = jogo) : (objs = jogo.o, vai chamar %.o)
$(JOGO_TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(JOGO_TARGET) $(OBJS)

$(TESTAR_TARGET): clean build_test
	./$(TESTAR_TARGET)
	#@gcov $(SRCS_ALL) > /dev/null
	#@echo "---CODE---"
	#@grep -Hn '#####' *.gcov || echo "Todas as linhas descobertas!"

# cflags = standard + gcov and define
build_test: $(OBJS_ALL)
	$(CC) $(CFLAGS) -o $(TESTAR_TARGET) $(OBJS_ALL) $(LDFLAGS)

# $< target (neste caso %.o) $@ primeira depend (neste caso %c)

#cflags
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -f $(OBJS_ALL) $(JOGO_TARGET) $(TESTAR_TARGET) *.gcno *.gcda *.gcov
