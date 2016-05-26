#include "MsgStruct.h"

// 内部类型和协议互转
void StruCmd2MCmd(struCommand& strucmd, AFPROC::MCommand& mcmd)
{
    mcmd.set_d_type(strucmd.type);
    mcmd.set_d_address(strucmd.addr);
    mcmd.set_cmd_class(strucmd.cmd_class);
    mcmd.set_cmd_func(strucmd.cmd_func);
    mcmd.set_execommand(strucmd.cmd);
    mcmd.set_dcode(strucmd.dcode);
    return;
}

void MCmd2StruCmd(AFPROC::MCommand& mcmd, struCommand& strucmd)
{
    strucmd.type = mcmd.d_type();
    strucmd.addr = mcmd.d_address();
    strucmd.cmd_class = mcmd.cmd_class();
    strucmd.cmd_func = mcmd.cmd_func();
    strucmd.cmd = mcmd.execommand();
    strucmd.dcode = mcmd.dcode();
    return;
}

void StruRequest2MRequest(struRequest& struReq, AFPROC::MRequest mReq)
{
    mReq.set_d_type(struReq.d_type);
    mReq.set_d_address(struReq.d_addr);
    mReq.set_cmd_class(struReq.cmd_class);
    mReq.set_cmd_func(struReq.cmd_func);
    mReq.set_execommand(struReq.cmd);
    mReq.set_dcode(struReq.dcode);
    mReq.set_m_body(struReq.body);
    mReq.set_token(struReq.token);
    mReq.set_seq_num(struReq.seq);
    return;
}

void MRequest2StruRequest(AFPROC::MRequest mReq, struRequest& struReq)
{
    struReq.d_type = mReq.d_type();
    struReq.d_addr = mReq.d_address();
    struReq.cmd_class = mReq.cmd_class();
    struReq.cmd_func = mReq.cmd_func();
    struReq.cmd = mReq.execommand();
    struReq.dcode = mReq.dcode();
    struReq.body = mReq.m_body();
    struReq.token = mReq.token();
    struReq.seq = mReq.seq_num();
    return;
}