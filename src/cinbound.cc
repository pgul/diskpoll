#include "cinbound.h"
#include <stdio.h>
#include <fstream.h>
#include "copyfile.h"
#include <ctype.h>

void CInbound::makevalidpath()
{
  if (!(strPath.charAt(strPath.Length()-1)==':'||
        strPath.charAt(strPath.Length()-1)=='\\'||
        strPath.charAt(strPath.Length()-1)=='/'))
    strPath+='/';
}

                                // MakeNewFileName checks if the filename
                                // passed as argument already does exist. If
                                // not, the same name is returned. If it does
                                // exist, a new filename is created. This way,
                                // we assure that received files never
                                // overwrite existing ones.
                                // May return an empty name if no name could be
                                // found.

static CString makeNewFilename(const CString& strName)
{
  CString str=strName;
  char *lastch;
  static char altExts[]="0123456789abcdefghijklmnopqrstuvwxyz";
  size_t length;

  lastch=altExts;
  length = str.Length();

  /* Rename ?UT-Packets to .PKT */
  if (length > 4)
    {
      if (toupper(str.charAt(length-1)) == 'T' &&
          toupper(str.charAt(length-2)) == 'U' &&
          toupper(str.charAt(length-4)) == '.')
        {
          str.setCharAt(length-3,'p');
          str.setCharAt(length-2,'k');
          str.setCharAt(length-1,'t');
        }
    }

  if (length)
    {
      while (1)
        {
          ifstream ifs(str,ios::binary|ios::in);
          if (!ifs)                  // file does not exist
            break;
          ifs.close();               // file did exist - find new name

          if (*lastch)
            str.charAt(length-1)=*(lastch++);
          else
            {
              str="";       // could not find a new name
              break;
            }
        }
    }

  return str;
}


CInbound::CInbound():strPath(){ makevalidpath(); }
CInbound::CInbound(const CString&r):strPath(r){ makevalidpath(); }

int CInbound::Receive(const CString& strFilename, int killflag)
{
  CString strName,strDestname;
  size_t index;
  int crc;
                                // find the corresponding directory name
  for (index=strFilename.Length();index>0;index--)
    if (strFilename.charAt(index-1)=='\\'||
        strFilename.charAt(index-1)=='/'||
        strFilename.charAt(index-1)==':')
      break;

  if (index<strFilename.Length()||strFilename.Length()==0)
    strName=strFilename.substr(index,strFilename.Length()-1);
  else
    return 0;                   // file not found ...

  strDestname=makeNewFilename(strPath+strName);
  if (!strDestname.Length())
    return 0;                   // no name for destination file could be found
  crc = copyfile(strDestname,strFilename);
  switch(crc)
    {
    case COPY_NOERR:
      if (killflag==KILL_KILL)
        remove(strFilename);
      if (killflag==KILL_TRUNC)
        {
          remove(strFilename);
          ofstream f(strFilename);
          f.close();
        }
      return RECERR_NOERROR;
    case COPY_NOTEXIST:
      return RECERR_NOTEXIST;
    }
  return RECERR_OTHER;
}
