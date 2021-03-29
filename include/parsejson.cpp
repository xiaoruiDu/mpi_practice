#include "parsejson.h"


ParseJson::ParseJson()
{
    return;
}

vector<agent_S> ParseJson::get_agent_json_data(std::string filename){

       // parse JSON file
    std::ifstream f_json(filename);
    json j1 = json::parse(f_json);

    // std::string dest = std::string( 3, '0').append("1");
    // cout << "dest:" << dest <<endl;

    int item_count = 0;
    int padding_zero = 3;
    for (int i=0; i<j1.size(); ++i) {
        agent_S agent_item;
        
        switch (to_string(item_count).size())
        {
        case 1:
            padding_zero = 3;
            break;
        case 2:
            padding_zero = 2;
            break;
        case 3:
            padding_zero = 1;
            break;
        case 4:
            padding_zero = 0;
            break;
        
        default:
            break;
        }


        string count_string =std::string( padding_zero, '0').append(to_string(item_count));
        agent_item.agent_id = j1[count_string]["agent_id"];
        agent_item.diameter = j1[count_string]["diameter"];
        agent_item.state_current.position.x = j1[count_string]["pos_x"];
        agent_item.state_current.position.y = j1[count_string]["pos_y"];
        agent_item.state_current.trajectory.x = j1[count_string]["traj_x"];
        agent_item.state_current.trajectory.y = j1[count_string]["traj_y"];
        agent_item.state_next = 0;
        // cout << "item:"<<item_count << "size:" <<to_string(item_count).size() << count_string <<endl;
        item_count += 1;
        agent_list.push_back(agent_item);
        // cout <<"agent_sizehaha:" <<agent_list.size()<<endl;
        
    }

    return agent_list;

}