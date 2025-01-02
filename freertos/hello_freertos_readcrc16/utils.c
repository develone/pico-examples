#ifndef _UTILS_H_
#define _UTILS_H_

static void
echo_key (char c);

static void
parse_cmd (void);
{
  // read line into buffer
  char *cp = cmd_buffer;
  char *cp_end = cp + sizeof (cmd_buffer);
  char c;
  do
    {
      c = getchar ();
      if (c == '\t')
	{
	  bool infirst = true;
	  for (char *p = cmd_buffer; p < cp; p++)
	    if ((*p == ' ') || (*p == ','))
	      {
		infirst = false;
		break;
	      }
	  if (infirst)
	    {
	      const char *p = search_cmds (cp - cmd_buffer);
	      if (p)
		{
		  while (*p)
		    {
		      *cp++ = *p;
		      echo_key (*p++);
		    }
		  *cp++ = ' ';
		  echo_key (' ');
		}
	    }
	  continue;
	}
      echo_key (c);
      if (c == '\b')
	{
	  if (cp != cmd_buffer)
	    {
	      cp--;
	      printf (" \b");
	      fflush (stdout);
	    }
	}
      else if (cp < cp_end)
	*cp++ = c;
    }
  while ((c != '\r') && (c != '\n'));
  // parse buffer
  cp = cmd_buffer;
  bool not_last = true;
  for (argc = 0; not_last && (argc < MAX_ARGS); argc++)
    {
      while ((*cp == ' ') || (*cp == ','))
	cp++;			// skip blanks
      if ((*cp == '\r') || (*cp == '\n'))
	break;
      argv[argc] = cp;		// start of string
      while ((*cp != ' ') && (*cp != ',') && (*cp != '\r') && (*cp != '\n'))
	cp++;			// skip non blank
      if ((*cp == '\r') || (*cp == '\n'))
	not_last = false;
      *cp++ = 0;		// terminate string
    }
  argv[argc] = NULL;
}

char *
full_path (const char *name)
{
  if (name == NULL)
    return NULL;
  if (name[0] == '/')
    {
      strcpy (path, name);
    }
  else if (curdir[0] == 0)
    {
      strcpy (path, "/");
      strcat (path, name);
    }
  else
    {
      strcpy (path, curdir);
      if (name[0])
	{
	  strcat (path, "/");
	  strcat (path, name);
	}
    }
  return path;
}

static void
xmodem_cb (uint8_t * buf, uint32_t len)
{
  if (lfs_file_write (&file, buf, len) != len)
    printf ("error writing file\n");
}