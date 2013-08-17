#include "error.h"

uint handle_error(error_t *e, uint line, char* file, errornumber_t en)
{
  error_t e_e = {stderr,file,line,0,error_error,false,false};

  if((en == error_error)&&(e!=NULL)&&(file!=NULL)){
    if(!e->silent)
      fprintf(stderr,"(error \"%s\" (# %d) (level %d))",error_string[en],en);
    return 0;
  } else if(e==NULL||file==NULL){
    e_e.level++;
    handle_error(&e_e,line,file,error_error);
    return 1;
  }else{
    e->line = line;
    e->file = file;
    e->error_enum = en;
  }

  if(!e->silent)
    fprintf(e->stderr_output, "(error (file \"%s\") (line %d) (msg \"%s\") (# %d) (level %d))",file,line,error_string[en],en,e->level);
  return 0;
}
