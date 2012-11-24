#ifndef CLI_H_
#define CLI_H_

#define cli_sd  ((BaseSequentialStream *)&SDDM)
#define cli_println(st) chprintf((cli_sd), ("%s\r\n"), (st))
#define cli_print(st)   chprintf((cli_sd), ("%s"),     (st))

#endif /* CLI_H_ */
