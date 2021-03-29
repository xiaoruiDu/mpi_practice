#include "id_provider.h"


 IDProvider::IDProvider(int id_prefix)
 {
     _id_prefix = id_prefix;
     _next_id = 0;


 }

id_package IDProvider::get_id(){

     int my_id = _next_id;
     _next_id += 1;
     
    id_package id;
    id._id_prefix = _id_prefix;
    id.my_id = my_id;

    return id;
         

 }

