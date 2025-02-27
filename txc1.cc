#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "spaningtree_m.h"

using namespace omnetpp;

int tic0[2] = {1,5};
int tic1[4] = {0,2,3,4};
int tic2[1] = {1};
int tic3[2] = {1,4};
int tic4[3] = {1,3,5};
int tic5[2] = {0,4};
void copy_id(char *dest, std::string source)
{
    for(int i = 0; i<4; i++)
    {
        dest[i] = source[i];
    }
}
class Txc1 : public cSimpleModule
{
  private:
    int hop_count = 0; // Đếm số hop từ current bridge tới root bridge
    int designated_port = -1; // cổng designated_port là số thứ tự của port nối tới designated bridge
    char root_id[4];     // root_id là ID của bridge mà current bridge đang tin là root
    int block_ports[5];
    /* block ports là một danh sách các ID của port mà current bridge không được forward meassge theo
    để tránh tạo loop */
  protected:
    virtual SpanTree *generateMessage(); // Hàm khởi tạo một bản tin configuration
    virtual void forwardMessage(SpanTree *msg, int out); // Hàm giúp bridge forward bản tin
    virtual void initialize() override;  // Hàm khởi tạo mạng và các bridge trong mạng
    virtual void handleMessage(cMessage *msg) override;
    /* Hàm dùng để xử lý bản tin mà bridge nhận được, hầu như tất cả các quy trình cập nhật cho bridge
     đều diễn ra trong hàm này*/
    virtual bool compareID(SpanTree *msg);
    /*So sánh id đầu vào với root id lưu trong object và tự cập nhật các thông số:
     root_id, hop_count, designated_port, block_ports
     Cần trả về giá trị boolean có forward measseage không*/
};

Define_Module(Txc1);

void Txc1::initialize()
{
   // Boot the process scheduling the initial message as a self-message.
    strcpy(this->root_id, getName());
    printf("Initialization here %s", this->root_id);
    printf("\n");
    for(int i = 0; i<5; i++)
    {
        this->block_ports[i] = -1;
    }
    char msgname[20];
    // Hàm getName() sẽ trả về ID của Bridge
    sprintf(msgname, "%s", getName());
    SpanTree *msg = generateMessage();
    int n = gateSize("out");
    printf("%d",&n);
    int outGateBaseId = gateBaseId("out");
    for (int i = 0; i < n; i++)
        send(i==n-1 ? msg : msg->dup(), outGateBaseId+i);
    printf("Done initialize!");
    int delta = 10;
    // Đoạn code này dùng để tự gửi cho sub-module một self-message
    // khi đó, module sẽ check, nếu message tới là một self-message thì cần kiểm tra liệu mình có phải là
    // root không, nếu là root thì sẽ tiếp tục generate message và forward ra tất cả các port
    SpanTree *msg_not_forward = new SpanTree("self_mess");
    scheduleAt(simTime() + delta, msg_not_forward);
    scheduleAt(simTime() + delta*2, msg_not_forward->dup());
    scheduleAt(simTime() + delta*3, msg_not_forward->dup());
    scheduleAt(simTime() + delta*4, msg_not_forward->dup());
    scheduleAt(simTime() + delta*5, msg_not_forward->dup());
}

void Txc1::handleMessage(cMessage *msg)
{
// Kiểm tra ở đây, nếu đủ điều kiện thì sẽ dừng không cho bridge forward nữa
    SpanTree *ttmsg = check_and_cast<SpanTree *>(msg);
//    bool forward = true;
    bool self_mess = (strcmp(ttmsg->getName(), "self_mess") == 0);
    printf("self_mess %s", self_mess?"true":"false");
    bool forward = false;
    if (self_mess == false)
    {
        forward = this->compareID(ttmsg);
        printf("Done comparing!\n");
    }
    printf("root now is %s\n", this->root_id);
    char src_id[4];
    strcpy(src_id, getName());
    bool is_root = false;
    printf("Current tic %s\n", src_id);
    printf("Current root %s\n", this->root_id);
    if (strcmp(src_id, this->root_id) == 0) is_root = true;
    printf("%s", is_root?"true":"false");
    int count_send = 0;
    if (forward == true && is_root == false && self_mess == false)
    {
        int n = gateSize("in");
        for (int i = 0; i<n; i++)
        {
           bool send = true;
           for(int j = 0; j <5; j++)
           {
               if (i == this->block_ports[j])
               {
                   send = false;
                   printf("block port is %d",i);
               }
           }
           if (send == true)
           {
               count_send ++;
               ttmsg->setSource_id(getName());
               ttmsg->setHopCount(this->hop_count);
               forwardMessage(ttmsg->dup(), i);
           }
        }
       if (count_send == 0) delete ttmsg;
    }
//    else if (is_root == true && self_mess == true)
    else if (strcmp(src_id, "tic0") == 0)
    {
    delete ttmsg;
    printf("Send out again from %d!", this->root_id);
    SpanTree *newmsg = generateMessage();
    int n = gateSize("out");
    int outGateBaseId = gateBaseId("out");
    for (int i = 0; i < n; i++)
      send(i==n-1 ? newmsg : newmsg->dup(), outGateBaseId+i);
    }
}

SpanTree *Txc1::generateMessage()
{
    // Produce source and destination addresses.
    char src_id[4];
    strcpy(src_id, getName());
    char msgname[20];
    sprintf(msgname, "tic-%s", src_id);
    // Create message object and set source and destination field.
    SpanTree *msg = new SpanTree(msgname);
    msg->setSource_id(src_id);
    msg->setRoot_id(this->root_id);
    msg->setHopCount(this->hop_count);
    return msg;
}

void Txc1::forwardMessage(SpanTree *msg, int out)
{
    EV << "Forwarding message " << msg << " on port out[" << out << "]\n";
    send(msg, "out", out);
}

bool Txc1::compareID(SpanTree *msg)
{
    printf("come to compareID\n");
    char mess_root_id[4];
    printf("%s",msg->getRoot_id());
    copy_id(mess_root_id,msg->getRoot_id());
    printf("%s \n", mess_root_id);
    int mess_root_id_num = int(mess_root_id[3]) - 48;
    printf("%d\n",mess_root_id_num);

    char mess_source_id[4];
    copy_id(mess_source_id, msg->getSource_id());
    int mess_source_id_num = int(mess_source_id[3]) - 48;
    printf("%d\n", mess_source_id_num);

    char current_source_id[4];
    strcpy(current_source_id,getName());
    int current_source_id_num = int(current_source_id[3]) - 48;
    printf("%d\n", current_source_id_num);

    int current_root_id_num = int(this->root_id[3]) - 48;
    printf("%d\n", current_root_id_num);
    int n = 0;
    int port_arr[4];
    switch (current_source_id_num)
    {
        case 0:
            n = sizeof(tic0)/sizeof(tic0[0]);
            for(int i = 0; i<n; i++)
            {
                port_arr[i] = tic0[i];
            }
            break;
        case 1:
            n = sizeof(tic1)/sizeof(tic1[0]);
            for(int i = 0; i<n; i++)
            {
                port_arr[i] = tic1[i];
            }
            break;
        case 2:
            n = sizeof(tic2)/sizeof(tic2[0]);
            for(int i = 0; i<n; i++)
            {
                port_arr[i] = tic2[i];
            }
            break;
        case 3:
            n = sizeof(tic3)/sizeof(tic3[0]);
            for(int i = 0; i<n; i++)
            {
                port_arr[i] = tic3[i];
            }
            break;
        case 4:
            n = sizeof(tic4)/sizeof(tic4[0]);
            for(int i = 0; i<n; i++)
            {
                port_arr[i] = tic4[i];
            }
            break;
        case 5:
            n = sizeof(tic5)/sizeof(tic5[0]);
            for(int i = 0; i<n; i++)
            {
                port_arr[i] = tic5[i];
            }
            break;
        default:
            printf("Error, using default!");
            return false;
    }
    if (current_root_id_num > mess_root_id_num)
    {
        strcpy(this->root_id, mess_root_id);
        printf("update root id to %s \n", this->root_id);
        this->hop_count ++;
        for (int i = 0; i <n; i++)
        {
             if (port_arr[i] == mess_root_id_num)
             {
                  this->designated_port = i;
                  break;
             }
        }
        for (int i = 0; i <5; i++)
        {
            if (this->block_ports[i] == this->designated_port) break;
            else if (this->block_ports[i] == -1)
            {
                this->block_ports[i] = this->designated_port;
                printf("block port is %d", this->block_ports[i]);
                break;
            }
        }
        return true;
    }
    else if (current_root_id_num == mess_root_id_num && msg->getHopCount()+1 < this->hop_count)
    {
        this->hop_count = msg->getHopCount()+1;

        for (int i = 0; i <n; i++)
        {
            if (port_arr[i] == mess_source_id_num)
            {
                this->designated_port = i;
                break;
            }
        }
        for (int i = 0; i <5; i++)
        {
            if (this->block_ports[i] == this->designated_port) break;
            else if (this->block_ports[i] == -1)
            {
                this->block_ports[i] = this->designated_port;
                break;
            }
        }
        return true;
    }
    else if (current_root_id_num == mess_root_id_num && msg->getHopCount() + 1 == this->hop_count && current_source_id_num > mess_source_id_num)
    {

        for (int i = 0; i <n; i++)
        {
            if (port_arr[i] == mess_source_id_num)
            {
                this->designated_port = i;
                break;
            }
        }
        for (int i = 0; i <5; i++)
        {
            if (this->block_ports[i] == this->designated_port) break;
            else if (this->block_ports[i] == -1)
            {
                this->block_ports[i] = this->designated_port;
                break;
            }
        }
        return true;
    }
    else if (current_root_id_num == mess_root_id_num && msg->getHopCount()+1 > this->hop_count)
    {
        for (int i = 0; i <n; i++)
        {
            if (port_arr[i] == mess_source_id_num)
            {
                for (int j = 0; j <5; j++)
                {
                    if (this->block_ports[j] == i) break;
                    else if (this->block_ports[j] == -1)
                    {
                        this->block_ports[j] = i;
                        break;
                    }
                }
                break;
            }
        }
        return false;
    }
    else if (current_root_id_num == mess_root_id_num && msg->getHopCount() + 1 == this->hop_count && current_source_id_num < mess_source_id_num)
    {
        for (int i = 0; i <n; i++)
        {
            if (port_arr[i] == mess_source_id_num)
            {
                for (int j = 0; j <5; j++)
                {
                    if (this->block_ports[j] == i) break;
                    else if (this->block_ports[j] == -1)
                    {
                        this->block_ports[j] = i;
                        break;
                    }
                }
                break;
            }
        }
        return false;
    }
    return false;
 }
