#pragma once
#define SAFEDEL(p) if(p!=NULL){delete p; p = NULL; }
#define SAFE_DELETE_ARRAY(p){if((p) != NULL){delete [](p);}}
#define SAFERALEASE(p)if(p!=NULL){p->Release(); p=NULL;}
#define SAFEDESTORY(p)if(p!=NULL){p->destory(); p=NULL;}
#define SAFE_UNLOCK(p)if(p!=NULL){p->unlock(); p=NULL;}