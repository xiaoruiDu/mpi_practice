#ifndef _ID_PROVIDER_H
#define _ID_PROVIDER_H

struct id_package
{
    int _id_prefix;
    int my_id;
};


class IDProvider{

    public:
        IDProvider(int id_prefix=-1);

        id_package get_id();

        int _id_prefix;
        int _next_id;



};





#endif //_ID_PROVIDER_H