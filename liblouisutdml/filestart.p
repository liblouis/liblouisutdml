  widechar outbuf[2 * BUFSIZE];
  widechar pagebuf[2 * BUFSIZE];
  if (!read_configuration_file
      (configFileName, logFileName, settingsString, mode))
    return 0;
  ud->outbuf = outbuf;
  ud->outlen = (sizeof (outbuf) / CHARSIZE) - 4;
  ud->pagebuf = pagebuf;
  ud->pagelen = (sizeof (pagebuf) / CHARSIZE) - 4;
