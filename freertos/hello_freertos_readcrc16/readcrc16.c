#include "readcrc16.h"
static void
j2k_cmd (void)
{
  if (check_mount (true))
    return;
	//in_fname is passed as argv[1]
	//out_fname is passed as argv[2]
	//Compression Ratio
	//Compress DeCompress
	printf ("%d in_fname = %s out_fname = %s CR %d C/D = %d \n", argc, argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
//lfsfile_close (&in);
}


echo_key (char c)
{
  putchar (c);
  if (c == '\r')
    putchar ('\n');
}


static void
parse_cmd (void)
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





static bool
check_name (void)
{
  if (argc > 1)
    return false;
  strcpy (result, "missing file or directory name");
  return true;
}







 






static void
unmount_cmd (void)
{
  if (check_mount (true))
    return;
  if (pico_unmount () != LFS_ERR_OK)
    {
      strcpy (result, "Error unmounting filesystem");
      return;
    }
  mounted = false;
  strcpy (result, "mounted");
}

static void
format_cmd (void)
{
  if (check_mount (false))
    return;
  printf ("are you sure (y/N) ? ");
  fflush (stdout);
  parse_cmd ();
  if ((argc == 0) || ((argv[0][0] | ' ') != 'y'))
    {
      strcpy (result, "user cancelled");
      return;
    }
  //if (format != LFS_ERR_OK)
    //strcpy (result, "Error formating filesystem");
  //strcpy (result, "formatted");
}

static void
status_cmd (void)
{
  if (check_mount (true))
    return;
  struct pico_fsstat_t stat;
  pico_fsstat(&stat);
  const char percent = 37;
  sprintf (result,
	   "\nflash base 0x%x, blocks %d, block size %d, used %d, total %u bytes, %1.1f%c used.\n",
	   fs_flash_base (), (int) stat.block_count, (int) stat.block_size,
	   (int) stat.blocks_used, stat.block_count * stat.block_size,
	   stat.blocks_used * 100.0 / stat.block_count, percent);
}



/*
static void
vi_cmd (void)
{
  if (check_mount (true))
    return;
  vi (screen_x, screen_y, argc - 1, argv + 1);
  strcpy (result, VT_CLEAR "\n");
}
*/
static bool
check_mount (bool need)
{
   if (mounted == need)
     return false;
   sprintf (result, "filesystem is %s mounted", (need ? "not" : "already"));
   return true;
}

static void
put_cmd (void)
{
  if (check_mount (true))
    return;
  if (check_name ())
    return;
  if (fs_file_open (&file, full_path (argv[1]), LFS_O_WRONLY | LFS_O_CREAT) <
      0)
    {
      strcpy (result, "Can't create file");
      return;
    }
  stdio_set_translate_crlf (&stdio_uart, false);
  xmodemReceive (xmodem_cb);
  stdio_set_translate_crlf (&stdio_uart, true);
  int pos = fs_file_seek (&file, 0, LFS_SEEK_END);
  fs_file_close (&file);
  sprintf (result, "\nfile transfered, size: %d\n", pos);
}

int
check_cp_parms (char **from, char **to, int copy)
{
  *from = NULL;
  *to = NULL;
  int rc = 1;
  do
    {
      if (argc < 3)
	{
	  strcpy (result, "need two names");
	  break;
	}
      *from = strdup (full_path (argv[1]));
      if (*from == NULL)
	{
	  strcpy (result, "no memory");
	  break;
	}
      if (copy)
	{
	  struct lfs_info info;
	  if (fs_stat (*from, &info) < 0)
	    {
	      sprintf (result, "%s not found", *from);
	      break;
	    }
	  if (info.type != LFS_TYPE_REG)
	    {
	      sprintf (result, "%s is a directory", *from);
	      break;
	    }
	}
      *to = strdup (full_path (argv[2]));
      if (*to == NULL)
	{
	  strcpy (result, "no memory");
	  break;
	}
      struct lfs_info info;
      if (fs_stat (*from, &info) < 0)
	{
	  sprintf (result, "%s not found", *from);
	  break;
	}
      if (fs_stat (*to, &info) >= 0)
	{
	  sprintf (result, "%s already exists", *to);
	  break;
	}
      rc = 0;
    }
  while (0);
  if (rc)
    {
      if (*from)
	free (*from);
      if (*to)
	free (*to);
    }
  return rc;
}

static void
mv_cmd (void)
{
  char *from;
  char *to;
  if (check_cp_parms (&from, &to, 0))
    return;
  if (fs_rename (from, to) < 0)
    sprintf (result, "could not rename %s to %s", from, to);
  else
    sprintf (result, "%s renamed to %s", from, to);
  free (from);
  free (to);
}

static void
cp_cmd (void)
{
  char *from;
  char *to;
  char *buf = NULL;
  if (check_cp_parms (&from, &to, 1))
    return;
  result[0] = 0;
  lfs_file_t in, out;
  do
    {
      buf = malloc (4096);
      if (buf == NULL)
	{
	  strcpy (result, "no memory");
	  break;
	}
      if (fs_file_open (&in, from, LFS_O_RDONLY) < 0)
	{
	  sprintf (result, "error opening %s", from);
	  break;
	}
      if (fs_file_open (&out, to, LFS_O_WRONLY | LFS_O_CREAT) < 0)
	{
	  sprintf (result, "error opening %s", from);
	  break;
	}
      int l = fs_file_read (&in, buf, 4096);
      while (l )
	{
	  if (fs_file_write (&out, buf, l) != l)
	    {
	      sprintf (result, "error writing %s", to);
	      break;
	    }
	  l = fs_file_read (&in, buf, 4096);
	}
    }
  while (false);
  fs_file_close (&in);
  fs_file_close (&out);
  if (buf)
    free (buf);
  if (!result[0])
    sprintf (result, "file %s copied to %s", from, to);
  free (from);
  free (to);

}

static void
get_cmd (void)
{
  if (check_mount (true))
    return;
  if (check_name ())
    return;
  if (fs_file_open (&file, full_path (argv[1]), LFS_O_RDONLY) < 0)
    {
      strcpy (result, "Can't open file");
      return;
    }
  uint32_t len = fs_file_seek (&file, 0,  LFS_SEEK_END);
  fs_file_rewind (&file);
  char *buf = malloc (len);
  if (buf == NULL)
    {
      strcpy (result, "not enough memory");
      goto err2;
    }
  if (fs_file_read (&file, buf, len) != len)
    {
      strcpy (result, "error reading file");
      goto err1;
    }
  stdio_set_translate_crlf (&stdio_uart, false);
  xmodemTransmit (buf, len);
  stdio_set_translate_crlf (&stdio_uart, true);
  printf ("\nfile transfered, size: %d\n", len);
err1:
  free (buf);
err2:
  fs_file_close (&file);
  strcpy (result, "transfered");
}

static void
mkdir_cmd (void)
{
  if (check_mount (true))
    return;
  if (check_name ())
    return;
  if ( fs_mkdir (full_path (argv[1])) < 0)
    {
      strcpy (result, "Can't create directory");
      return;
    }
  sprintf (result, "%s created", full_path (argv[1]));
}

static void
rm_cmd (void)
{
  if (check_mount (true))
    return;
  if (check_name ())
    return;
  // lfs won't remove a non empty directory but returns without error!
  struct lfs_info info;
  char *fp = full_path (argv[1]);
  if (fs_stat (fp, &info) < 0)
    {
      sprintf (result, "%s not found", full_path (argv[1]));
      return;
    }
  int isdir = 0;
  if (info.type == LFS_TYPE_DIR)
    {
      isdir = 1;
      lfs_dir_t dir;
      fs_dir_open (&dir, fp);
      int n = 0;
      while (fs_dir_read (&dir, &info))
	if ((strcmp (info.name, ".") != 0) && (strcmp (info.name, "..") != 0))
	  n++;
      fs_dir_close (&dir);
      if (n)
	{
	  sprintf (result, "directory %s not empty", fp);
	  return;
	}
    }
  if (fs_remove (fp) < 0)
    strcpy (result, "Can't remove file or directory");
  sprintf (result, "%s %s removed", isdir ? "directory" : "file", fp);
}

static void
mount_cmd (void)
{
  char c;
  if (check_mount (false))
    return;
    if (pico_mount((c | ' ') == 'y') != LFS_ERR_OK) {
        printf("Error mounting FS\n");
        exit(-1);
    }
  mounted = true;
  strcpy (result, "mounted");
}

static void
ls_cmd (void)
{
  if (check_mount (true))
    return;
  if (argc )
    full_path (argv[1]);
  else
    full_path ("");
  lfs_dir_t dir;
  if (fs_dir_open (&dir, path) < 0)
    {
      strcpy (result, "not a directory");
      return;
    }
  printf ("\n");
  struct lfs_info info;
  while (fs_dir_read (&dir, &info) )
    if (strcmp (info.name, ".") && strcmp (info.name, ".."))
      if (info.type == LFS_TYPE_DIR)
	printf (" %7d [%s]\n", info.size, info.name);
  fs_dir_rewind (&dir);
  while (fs_dir_read (&dir, &info) )
    if (strcmp (info.name, ".") && strcmp (info.name, ".."))
      if (info.type == LFS_TYPE_REG)
	printf (" %7d %s\n", info.size, info.name);
  fs_dir_close (&dir);
  result[0] = 0;
}

static void
test_pnmio_cmd (void)
{
  if (check_mount (true))
    return;
	printf ("%d %s\n", argc, argv[1]);
    char ch;
	int ncols;
	int *ptrncols;
	int nrows;
	int *ptrnrows;
	int *maxval;
	char *fname;
	unsigned char *img;
	int i,j;
	short int val; 
 	ncols=320;
	nrows=240;
	ptrncols = &ncols;
	ptrnrows = &nrows;
	printf ("%d %s \n", argc, argv[1]);
	//fname is passed as argv[1]
	printf("ptrncols = 0x%x,ptrnrows = 0x%x \n",ptrncols, ptrnrows);
	lfs_file_t in, out;
    if (fs_file_open(&in, argv[1], LFS_O_RDONLY) < 0)
  		printf("error in open\n");        
 
/*	
	//img = pgmReadFile(fname,NULL,ptrncols, ptrnrows);
	printf("0x%x,%d 0x%x %d \n",ptrncols,ncols, ptrnrows, nrows);
	for(j=0;j<nrows;j++) {
		for (i=0;i<ncols;i++) {
			val = *img;
			printf("%d j=  %d i=  %d \n",j,i,val);
		img++;
	}
}
*/
//lfsfile_close (&in);
}

static void
lsklt_cmd (void)
{
  if (check_mount (true))
    return;

  ptrs.w = 64;
  ptrs.h = 64;
	tc = KLTCreateTrackingContext ();
  //printf("tc 0x%x\n",tc);
  fl = KLTCreateFeatureList (nFeatures);
  ptrs.out_buf = ptrs.inpbuf + imgsize;
  ptrs.inp_buf = ptrs.inpbuf;
  ncols = 64;
  nrows = 64;
  printf ("%d %s %s %s\n", argc, argv[1], argv[2], argv[3]);
	ptrs.fwd_inv = &ptrs.fwd;
	*ptrs.fwd_inv = 1;
  
  lfs_file_t in, out;


  //if (lfsfile_open(&in, argv[1], LlfsO_RDONLY) < 0)
  //printf("error in open\n");        

  printf ("%d\n", fs_file_open (&in, argv[1], LFS_O_RDONLY));
  int l = fs_file_size (&in), charcnt = 0, charsent = 0;
  int ii = 0, jj = 0, flag;
  char *bufptr;
   
  //char *outstrptr;
  char *buf = malloc (l + 1);
	char *outstr = malloc (75 + 1);
  fs_file_read (&in, buf, l);
/*
Read the pgm header
P5                                                                              
# Created by GIMP version 2.10.8 PNM plug-in                                    
48                                                                              
64 64                                                                           
255
*/
  for (ii = 0; ii < 3; ii++)
    printf ("%c", buf[ii]);
  flag = 1;
  while (flag)
    {
      if (buf[ii] != 10)
	{
	  printf ("%c", buf[ii]);
	  ii++;
	}
      else
	flag = 0;
    }
  ii++;
  //printf("\n%d\n",ii);
  flag = 1;
  while (flag)
    {
      if (buf[ii] != 10)
	{
	  printf ("%c", buf[ii]);
	  ii++;
	}
      else
	flag = 0;
    }
  printf ("\n");
  ii++;

  flag = 1;
  while (flag)
    {
      if (buf[ii] != 10)
	{
	  printf ("%c", buf[ii]);
	  ii++;
	}
      else
	flag = 0;
    }
  printf ("\n");
  ptrs.inp_buf = ptrs.inpbuf;
  ii++;
  bufptr = &buf[ii];
  for (jj = 0; jj < 64; jj++)
    {
      for (ii = 0; ii < 64; ii++)
	{
	  //printf("%d ",*bufptr);
	  *ptrs.inp_buf = (unsigned short int) *bufptr;
	  printf ("%d ", *ptrs.inp_buf);
	  bufptr++;
	  ptrs.inp_buf++;
	}
    }
  printf ("\n");

  ptrs.inp_buf = ptrs.inpbuf;
  printf ("opening a file to write the results\n");
  //if (lfsfile_open(&fd, argv[2], LlfsO_WRONLY) < 0)
  // printf("error in open\n"); 
  printf ("need to copy the data received from host to img1\n");
  img1 = inpbuf;
  img2 = &inpbuf[4096];

  printf ("img1 = 0x%x img2 = 0x%x\n", img1, img2);
  for (i = 0; i < ncols * nrows; i++)
    {
      img1[i] = ptrs.inp_buf[i];
      //img2[i+4096] = img1[i]; 
      if (i < 5)
	printf ("%d img1 %d ptrs.buf %d \n", i, img1[i], ptrs.inp_buf[i]);
      if (i )
	printf ("%d img1 %d ptrs.buf %d \n", i, img1[i], ptrs.inp_buf[i]);
    }
  printf ("need to copy the data from img1 to img2\n");
  for (i = 0; i < ncols * nrows; i++)
    {
      *img2 = *img1;
      if (i < 5)
	printf ("%d img2 %d img1 %d \n", i, *img2, *img1);
      if (i )
	printf ("%d img2 %d img1 %d \n", i, *img2, *img1);
      img2++;
      img1++;
    }
  ptrs.inp_buf = ptrs.inpbuf;
  img1 = inpbuf;
  img2 = &inpbuf[4096];

  //printf("img1 = 0x%x img2 = 0x%x\n",img1, img2);
	img1 = &inpbuf[0];
	img2 = &inpbuf[4096];

  if(atoi(argv[3])==1) {
		printf("klt\n");

		KLTSelectGoodFeatures (tc, img1, ncols, nrows, fl);

  	//printf("\nIn first image:\n");
  	for (i = 0; i < fl->nFeatures; i++)
    	{
      	printf ("Feature #%d:  (%f,%f) with value of %d\n",
        	 i, fl->feature[i]->x, fl->feature[i]->y,
         	fl->feature[i]->val); 

      	/*charsent = sprintf (outstr, "Feature #%d:  (%f,%f) with value of %d",
			  	i, fl->feature[i]->x, fl->feature[i]->y,
			  	fl->feature[i]->val);*/

      	//*outstr = *outstr + charsent;
      	//charcnt = charcnt + charsent;
    	}
  	//*outstr = *outstr - charcnt;
  	printf ("this is the string %d %s", charcnt, outstr);
	}
  else
	{
		printf("lifting step\n");
		ptrs.inp_buf = ptrs.inpbuf;
    printf("%d 0x%x 0x%x 0%x \n",ptrs.w, ptrs.inp_buf, ptrs.out_buf, *ptrs.fwd_inv);
		lifting (ptrs.w, ptrs.inp_buf, ptrs.out_buf, ptrs.fwd_inv);
		for(nrows=0;nrows<64;nrows++) {
			for(ncols=0;ncols<64;ncols++) {
				printf("%d ",ptrs.inp_buf[offset]);
				offset++;
			}
			printf("\n");
		}
	}
  free (buf);
	free (outstr);
  //free(&fd);
  fs_file_close (&in);
  printf ("%d \n", fs_file_open (&out, argv[2], LFS_O_WRONLY | LFS_O_CREAT));
  //lfsfile_write (&out, outstr, charsent);
  fs_file_close (&out);
}

static void
quit_cmd (void)
{
  // release any resources we were using
  if (mounted)
    fs_unmount ();
  strcpy (result, "");
  run = false;
}

int
read_tt (char *head, char *endofbuf, char *topofbuf)
{

  int i, numtoread = 64;
  unsigned char CRC;

  //printf("0x%x 0x%x 0x%x \n",ptrs.head,ptrs.endofbuf,ptrs.topofbuf);
  for (i = 0; i < numtoread; i++)
    {

      *ptrs.head = getchar ();
      ptrs.head =
	(char *) bump_head (ptrs.head, ptrs.endofbuf, ptrs.topofbuf);
    }

  CRC = crc16_ccitt (tt, numtoread);
  //printf("0x%x\n",CRC);
  //for(i=0;i<numtoread;i++) bump_tail(ptrs.head,ptrs.endofbuf,ptrs.topofbuf);
  //for(i=0;i<numtoread;i++) printf("%c",tt[i]);


  //printf("\n");


  //printf("0x%x 0x%x 0x%x \n",ptrs.head,ptrs.endofbuf,ptrs.topofbuf);
  //printf("CRC = 0x%x\n",CRC);

  return (1);
}








void
MyFunction (void)
{
  StreamBufferHandle_t xStreamBuffer;
  const size_t xTriggerLevel = 1;
  xStreamBuffer = xStreamBufferCreateStatic (sizeof (ucBufferStorage),
					     xTriggerLevel,
					     ucBufferStorage,
					     &xStreamBufferStruct);
}


void
vASendStream (StreamBufferHandle_t DynxStreamBuffer)
{
  size_t xByteSent;
  uint8_t ucArrayToSend = (0, 1, 2, 3);

  /*numbytes2 29 rdnumbytes1 30
     if the string is uncommneted */
  char *pcStringToSend = "String To Send String To Send";

  /*numbytes2 14 rdnumbytes1 15
     if the string is uncommneted */
  //char *pcStringToSend ="String To Send";

  const TickType_t x100ms = pdMS_TO_TICKS (100);

  xByteSent = xStreamBufferSend (DynxStreamBuffer, (void *) ucArrayToSend,
				 sizeof (ucArrayToSend), x100ms);
  numbytes1 = xByteSent;

  if (xByteSent != sizeof (ucArrayToSend))
    {

    }
  xByteSent = xStreamBufferSend (DynxStreamBuffer, (void *) pcStringToSend,
				 strlen (pcStringToSend), 0);
  numbytes2 = strlen (pcStringToSend);

  if (xByteSent != strlen (pcStringToSend))
    {

    }
}


void
vAReadStream (StreamBufferHandle_t xStreamBuffer)
{
  int i;
  uint8_t ucRXData[40];
  size_t xRecivedBytes;
  const TickType_t xBlockTime = pdMS_TO_TICKS (20);
  pucRXData = &ucRXData;
  xRecivedBytes = xStreamBufferReceive (DynxStreamBuffer,
					(void *) ucRXData, sizeof (ucRXData),
					xBlockTime);
  rdnumbytes1 = xRecivedBytes;
  i = 0;
  if (xRecivedBytes > 0)
    {
      printf ("%d ", ucRXData[i]);
      i++;
    }
}
