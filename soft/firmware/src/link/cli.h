#ifndef CLI_H_
#define CLI_H_

/**
 * Shell command structure
 */
typedef struct ShellCmd_t ShellCmd_t;

/**
 * Fucnction executing command job.
 */
typedef Thread* (*cmdfunction_t)(int argc, const char * const * argv, const ShellCmd_t *cmdarray);


struct ShellCmd_t{
  /**
   * Printable command name. Must be zero terminated string
   */
  char *name;
  /**
   * Function binded to command
   */
  const cmdfunction_t func;
  /**
   * Pointer to structure with subcommands. May be NULL.
   */
  const ShellCmd_t *subcmd;
};

void cli_print(const char *str);
void cli_println(const char *str);
void cli_print_long(const char * str, int n, int nres);
char get_char (void);

Thread* SpawnShellThreads(SerialDriver *sdp_cli);
void KillShellThreads(void);

#endif /* CLI_H_ */
