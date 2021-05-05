
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "op-common.h"
#include "op-class.h"

#include "emu-mem-io.h"
#include "emu-proc.h"
#include "emu-serial.h"
#include "emu-timer.h"
#include "emu-int.h"
#include "emu-con.h"

#include "op-exec.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#define MAINLOOP_TIMER 10000
static int mainloop_count = 0;

static int file_load (addr_t start, char * path)
	{
	int err = -1;
	int fd = -1;

	while (1)
		{
		// Load binary image file

		fd = open (path, O_RDONLY);
		if (fd < 0)
			{
			puts ("fatal: cannot open file");
			break;
			}

		off_t size = lseek (fd, 0, SEEK_END);
		if (size <= 0)
			{
			puts ("fatal: empty file");
			break;
			}

		printf ("info: file size=%lXh\n", size);
		if (start + size > MEM_MAX)
			{
			puts ("fatal: file too big");
			break;
			}

		lseek (fd, 0, SEEK_SET);

		byte_t * buf = mem_get_addr (start);
		ssize_t count = read (fd, buf, size);
		if (count != size)
			{
			puts ("fatal: incomplete file read");
			break;
			}

		puts ("success: file loaded");
		err = 0;
		break;
		}

	// Cleanup

	if (fd >= 0)
		{
		close (fd);
		fd = -1;
		}

	return err;
	}


//------------------------------------------------------------------------------
// Debug procedure
//------------------------------------------------------------------------------

static op_desc_t _op_desc;

static int _break_code_addr_count = 0;
static addr_t * _break_code_addr = NULL;
static addr_t _break_step_over_code_addr = -1;

static int _flag_trace  = 0;  // trace next instruction
// FIXME: used by signal handler in character backend
/*static*/ int _flag_prompt = 0;  // prompt for debug command
static int _flag_exec   = 1;  // execute next instruction
static int _flag_exit   = 0;  // exit main loop


static int debug_proc ()
	{
	int err = 0;

	while (1)
		{
		if (_flag_trace)
			{
			// Print next instruction before execution

			printf ("%.4hX:%.4hX  ", seg_get (SEG_CS), reg16_get (REG_IP));
			print_column (op_code_str, 3 * OPCODE_MAX + 1);
			print_op (&_op_desc);
			putchar ('\n');
			}

		if (_flag_prompt)
			{
			// Get user command
			// FIXME: use safer input function

			con_normal();
			char command [8];
			if (!_flag_trace) putchar ('\n');
			putchar ('>');
			fflush(stdout);
			char * res = fgets (command, 8, stdin);
			if (!res) {
				err = -1;
				break;
				}

			con_raw();

			switch (command [0])
				{
				// Dump stack

				case 's':
					stack_print ();
					_flag_trace = 0;
					_flag_exec = 0;
					break;

				// Print registers

				case 'r':
					regs_print ();
					_flag_trace = 0;
					_flag_exec = 0;
					break;

				// Step over

				case 'p':
					_break_step_over_code_addr = addr_seg_off (op_code_seg, op_code_off);
					_flag_trace = 0;
					_flag_prompt = 0;
					break;

				// Trace one step

				case 't':
				case '\n':
					_flag_trace = 1;
					_flag_prompt = 1;
					break;

				// Continue tracing

				case 'c':
					_flag_trace = 1;
					_flag_prompt = 0;
					break;

				// Go (keep breakpoints)

				case 'g':
					_flag_trace = 0;
					_flag_prompt = 0;
					break;

				// Quit

				case 'q':
					_flag_exec = 0;
					_flag_exit = 1;
					break;

				// Interrupt (only timer for now)

				case 'i':
					_flag_exec = 0;
					err = exec_int (0x08);  // timer 0 interrupt
					if (err) puts ("error: timer interrupt");
					break;

				}  // command switch
			}  // _flag_prompt

		break;
		}

	return err;
	}


//------------------------------------------------------------------------------
// Processor procedure
//------------------------------------------------------------------------------

int info_level = 0;

static void cpu_proc (void)
	{
	int err;
	int bp_idx;

	static word_t last_seg = 0xFFFF;
	static word_t last_off = 0xFFFF;
	static word_t next_off;

	// Handle interrupt request

	if (_int_cpu && flag_get (FLAG_IF) && rep_none () && seg_none ())
		{
		byte_t vect;
		err = int_ack (&vect);
		assert (!err);
		err = exec_int (vect);
		assert (!err);
		}

	// Decode next instruction

	op_code_seg = seg_get (SEG_CS);
	op_code_off = reg16_get (REG_IP);

	// Code breakpoint test

	for (bp_idx = 0; bp_idx < _break_code_addr_count; bp_idx++)
		{
		if (addr_seg_off (op_code_seg, op_code_off) == _break_code_addr[bp_idx])
			{
			printf ("info: code breakpoint %d hit\n", bp_idx);
			_flag_trace = 1;
			_flag_prompt = 1;
			}
		}

	if (addr_seg_off (op_code_seg, op_code_off) == _break_step_over_code_addr)
		{
		puts ("info: step-over breakpoint hit");
		_flag_trace = 1;
		_flag_prompt = 1;
		}

	// Optimize: no twice decoding of the same instruction
	// Example: REPeated or LOOP on itself

	if (op_code_seg != last_seg || op_code_off != last_off)
		{
		last_seg = op_code_seg;
		last_off = op_code_off;

		memset (&_op_desc, 0, sizeof _op_desc);

		err = op_decode (&_op_desc);
		if (err)
			{
			puts ("\nerror: unknown opcode");
			_flag_trace = 1;
			_flag_prompt = 1;
			_flag_exec = 0;
			}

		// Suspicious operation on null opcodes

		if (op_code_null)
			{
			puts ("\nerror: suspicious null opcodes");

			op_code_null = 0;

			_flag_trace = 1;
			_flag_prompt = 1;
			_flag_exec = 0;
			}

		next_off = op_code_off;
		}
	else
		{
		// Already decoded
		// Move directly to next instruction

		op_code_off = next_off;
		}

	// Debug procedure
	// After decoding the next instruction
	// Before executing that instruction

	err = debug_proc ();

	// Execute operation

	if (_flag_exec)
		{
		int trace_before = flag_get (FLAG_TF);

		reg16_set (REG_IP, op_code_off);

		err = op_exec (&_op_desc);
		if (err)
			{
			puts (err < 0 ? "error: execute operation" : "warning: paused (HLT)");
			_flag_trace = 1;
			_flag_prompt = 1;
			if (err < 0) reg16_set (REG_IP, last_off);
			}
		else
			{
			// Repeat the operation if prefixed

			if (rep_active ())
				{
				reg16_set (REG_IP, last_off);
				}
			else
				{
				seg_reset ();
				}

			// Trace the operation if no prefix

			if (rep_none () && seg_none () && trace_before && flag_get (FLAG_TF))
				{
				err = exec_int (0x01);  // trace interrupt
				if (err)
					{
					puts ("fatal: trace interrupt");
					_flag_exit = 1;
					}
				}
			}  // op_exec
		}  // flag_exec

	// Data breakpoint test
	// FIXME: break before executing data access

	if (_break_data_flag)
		{
		puts ("info: data breakpoint hit");
		_break_data_flag = 0;
		_flag_trace = 1;
		_flag_prompt = 1;
		}

	// INT3 breakpoint test

	if (_break_int_flag)
		{
		puts ("info: INT3 breakpoint hit");
		_break_int_flag = 0;
		_flag_trace = 1;
		_flag_prompt = 1;
		}
	}


//------------------------------------------------------------------------------
// Command line
//------------------------------------------------------------------------------

static void usage (char * argv0)
	{
	printf ("usage: %s [options]\n\n", argv0);
	puts ("  -w <address>         load address");
	puts ("  -f <path>            file path");
	puts ("  -I <path>            disk image path");
	puts ("  -x <segment:offset>  execute address");
	puts ("  -c <address>         code breakpoint address");
	puts ("  -d <address>         data breakpoint address");
	puts ("  -t                   trace mode");
	puts ("  -i                   interactive mode");
	puts ("  -p                   program mode");
	puts ("  -v <level>           verbose info level");
	}


int command_line (int argc, char * argv [])
	{
	int err = 0;

	char opt;
	int file_loaded = 0;
	addr_t file_address = -1;
	char * file_path = NULL;
	char * disk_image_path = NULL;
	addr_t new_bp = -1;
	addr_t * new_array = NULL;

	while (1)
		{
		opt = getopt (argc, argv, "w:f:I:x:c:d:v:tip");
		if (opt < 0 || opt == '?') break;

		switch (opt)
			{
			case 'w':  // load address
				if (sscanf (optarg, "%lx", &file_address) != 1)
					{
					puts ("error: bad load address");
					}
				else
					{
					printf ("info: load address %.5lXh\n", file_address);
					}

				break;

			case 'f':  // file path
				file_path = optarg;
				printf ("info: load file %s\n", file_path);
				break;

			case 'I':  // disk image file path
				disk_image_path = optarg;
				break;

			// Execution address:
			// used to override the default CS:IP at reset

			case 'x':  // execute address
				if (sscanf (optarg, "%hx:%hx", &op_code_seg, &op_code_off) != 2)
					{
					puts ("error: bad execute address");
					}
				else
					{
					printf ("info: execute address %.4hXh:%.4hXh\n", op_code_seg, op_code_off);

					seg_set (SEG_CS, op_code_seg);
					reg16_set (REG_IP, op_code_off);
					}

				break;

			case 'c':  // code breakpoint address
				if (sscanf (optarg, "%lx", &new_bp) != 1)
					{
					puts ("error: bad code breakpoint address");
					}
				else
					{
					printf ("info: code breakpoint address %.5lXh\n", new_bp);
					new_array = realloc(_break_code_addr, (_break_code_addr_count + 1)*sizeof(*_break_code_addr));
					if (!new_array)
						{
						puts("error: out of memory allocating new code breakpoint");
						}
					else
						{
						_break_code_addr = new_array;
						_break_code_addr_count++;
						_break_code_addr[_break_code_addr_count-1] = new_bp;
						}
					}
				break;

			case 'd':  // data breakpoint address
				if (sscanf (optarg, "%lx", &_break_data_addr) != 1)
					{
					puts ("error: bad data breakpoint address");
					}
				else
					{
					printf ("info: data breakpoint address %.5lXh\n", _break_data_addr);
					}

				break;

			case 't':  // trace mode
				_flag_trace = 1;
				break;

			case 'i':  // interactive mode
				_flag_trace = 1;
				_flag_prompt = 1;
				break;

			case 'v':  // verbose output level
				info_level = atoi(optarg);
				break;

			// Program mode:
			// used when running a stand-alone executable
			// in the tiny memory model where CS=DS=ES=SS

			case 'p':
				seg_set (SEG_DS, seg_get (SEG_CS));
				seg_set (SEG_ES, seg_get (SEG_CS));
				seg_set (SEG_SS, seg_get (SEG_CS));
				break;

			}  // option switch

		if (file_address != -1 && file_path)
			{
			if (!file_load (file_address, file_path))
				{
				file_loaded = 1;
				}

			file_path = NULL;
			file_address = -1;
			}

		if (disk_image_path)
			{
			if (!rom_image_load (disk_image_path))
				{
				file_loaded = 1;
				}

			disk_image_path = NULL;
			}

		}  // option loop

	// Force interactive mode if no file loaded
	// to allow automatic testing without file

	if (!file_loaded) {
		_flag_trace = 1;
		_flag_prompt = 1;
		}

	if (opt == '?' || optind != argc)
		{
		usage (argv [0]);
		err = -1;
		}

	return err;
	}


//------------------------------------------------------------------------------
// Main loop
//------------------------------------------------------------------------------

int main (int argc, char * argv [])
	{
	int err = 0;

#ifdef __EMSCRIPTEN__
	static char *av[] = { 0, "-w", "0xe0000", "-f", "Image", "-w", "0x80000", "-f", "romfs.bin", 0};
	argc = sizeof(av) / sizeof(av[1]) - 1;
	argv = av;
#endif

	while (1)
		{
		mem_io_reset ();
		proc_reset ();

		// LUTs auto check

		err = check_exec ();
		if (err) {
			puts ("fatal: auto check");
			break;
			}

		// Process command line

		err = command_line (argc, argv);
		if (err) break;

		rom_init ();     // ROM stub initialization
		int_init ();     // PIC initialization
		timer_init ();   // PIT initialization
		serial_init ();  // Serial port initialization
		con_init ();     // Console initialization

		op_code_base = mem_get_addr (0);

		while (!_flag_exit)
			{
			// Animate emulated devices

			// Idea there is to have one thread per device
			// even if using only one thread for all

			timer_proc ();

			err = serial_proc ();
			if (err) break;

			// TODO: optimize PIC processing with single pass
			// int_proc ();

			cpu_proc ();

			// TODO: move that logic in the console procedure

			if (++mainloop_count >= MAINLOOP_TIMER)
				{
				if (con_proc ())
					_flag_exit = 1;

#ifdef __EMSCRIPTEN__

				// Actually an asynchronous function
				// https://github.com/mfld-fr/emu86/issues/32#issuecomment-830690460

				// Unwind the stack and return to browser
				emscripten_sleep(1);
				// Browser called back and stack was rewinded

#endif

				mainloop_count = 0;
				}
			}

		break;
		}

	// Cleanup
	if (_break_code_addr_count)
		{
		free(_break_code_addr);
		}

	rom_term ();
	con_term ();
	serial_term ();

	return (err >= 0) ? EXIT_SUCCESS : EXIT_FAILURE;
	}

//------------------------------------------------------------------------------
