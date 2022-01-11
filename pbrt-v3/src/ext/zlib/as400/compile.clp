







             PGM



             DCL        VAR(&SRCLIB) TYPE(*CHAR) LEN(10) +
                          VALUE('ZLIB')
             DCL        VAR(&SRCFILE) TYPE(*CHAR) LEN(10) +
                          VALUE('SOURCES')
             DCL        VAR(&CTLFILE) TYPE(*CHAR) LEN(10) +
                          VALUE('TOOLS')

             DCL        VAR(&MODLIB) TYPE(*CHAR) LEN(10) +
                          VALUE('ZLIB')

             DCL        VAR(&SRVLIB) TYPE(*CHAR) LEN(10) +
                          VALUE('LGPL')

             DCL        VAR(&CFLAGS) TYPE(*CHAR) +
                          VALUE('OPTIMIZE(40)')

             DCL        VAR(&TGTRLS) TYPE(*CHAR) +
                          VALUE('V5R3M0')




             DCL        VAR(&CMDLEN) TYPE(*DEC) LEN(15 5) VALUE(300)
             DCL        VAR(&CMD) TYPE(*CHAR) LEN(512)
             DCL        VAR(&FIXDCMD) TYPE(*CHAR) LEN(512)




             CHGVAR     VAR(&FIXDCMD) VALUE('CRTCMOD' *BCAT &CFLAGS *BCAT      +
                        'SYSIFCOPT(*IFS64IO)' *BCAT                            +
                        'DEFINE(''_LARGEFILE64_SOURCE''' *BCAT                 +
                        '''_LFS64_LARGEFILE=1'') TGTRLS(' *TCAT &TGTRLS *TCAT  +
                        ') SRCFILE(' *TCAT &SRCLIB *TCAT '/' *TCAT             +
                        &SRCFILE *TCAT ') MODULE(' *TCAT &MODLIB *TCAT '/')


             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'ADLER32)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)

             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'COMPRESS)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)

             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'CRC32)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)

             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'DEFLATE)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)

             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'GZCLOSE)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)

             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'GZLIB)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)

             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'GZREAD)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)

             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'GZWRITE)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)

             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'INFBACK)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)

             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'INFFAST)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)

             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'INFLATE)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)

             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'INFTREES)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)

             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'TREES)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)

             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'UNCOMPR)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)

             CHGVAR     VAR(&CMD) VALUE(&FIXDCMD *TCAT 'ZUTIL)')
             CALL       PGM(QCMDEXC) PARM(&CMD &CMDLEN)




             CRTSRVPGM  SRVPGM(&SRVLIB/ZLIB) +
                          MODULE(&MODLIB/ADLER32     &MODLIB/COMPRESS    +
                                 &MODLIB/CRC32       &MODLIB/DEFLATE     +
                                 &MODLIB/GZCLOSE     &MODLIB/GZLIB       +
                                 &MODLIB/GZREAD      &MODLIB/GZWRITE     +
                                 &MODLIB/INFBACK     &MODLIB/INFFAST     +
                                 &MODLIB/INFLATE     &MODLIB/INFTREES    +
                                 &MODLIB/TREES       &MODLIB/UNCOMPR     +
                                 &MODLIB/ZUTIL)                          +
                          SRCFILE(&SRCLIB/&CTLFILE) SRCMBR(BNDSRC)       +
                          TEXT('ZLIB 1.2.8') TGTRLS(&TGTRLS)

             ENDPGM
