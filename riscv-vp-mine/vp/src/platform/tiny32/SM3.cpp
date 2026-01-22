#include "SM3.h"

sc_uint<32> V[8];
sc_uint<32> T[64];
sc_uint<32> W68[68];
sc_uint<32> W64[64];
sc_uint<32> V_buf[8];
sc_uint<32> SS1 = 0, SS2 = 0, TT1 = 0, TT2 = 0;
sc_uint<32> A=0, B=1, C=2, D=3, E=4, F=5, G=6, H=7;
sc_biguint<256> V_finish;

int CF_count;
int W68_count;
int W64_count;
int V_ready_out_count;
int opt;

sc_uint<32> IV[8] = {
    0x7380166F,
    0x4914B2B9,
    0x172442D7,
    0xDA8A0600,
    0xA96F30BC,
    0x163138AA,
    0xE38DEE4D,
    0xB0FB0E4E
};

void T_init() {
    for(int j = 0; j < 16; j++)
        T[j] = 0x79CC4519;

    for(int j = 16; j < 64; j++)
        T[j] = 0x7A879D8A;

    return;
}

sc_uint<32> shift_SM3(sc_uint<32> data, int length) {
	sc_uint<32> result;
	result = (data << length) ^ (data >> (32-length));
	return result;
}

sc_uint<32> FF(sc_uint<32> X, sc_uint<32>Y, sc_uint<32> Z, sc_uint<32> j) {
    if(j>=0 && j<=15)
        return (X ^ Y ^ Z);
    else if(j>=16 && j<=63)
        return ((X & Y) | (X & Z) | (Y & Z));

    return 0;
}

sc_uint<32> GG(sc_uint<32> X, sc_uint<32> Y, sc_uint<32> Z, sc_uint<32> j) {
    if(j>=0 && j<=15)
        return (X ^ Y ^ Z);
    else if(j>=16 && j<=63)
        return ((X & Y) | ((~X) & Z));

    return 0;
}

sc_uint<32> P0(sc_uint<32> X) {
    return (X ^ shift_SM3(X, 9) ^ shift_SM3(X, 17));
}

sc_uint<32> P1(sc_uint<32> X) {
    return (X ^ shift_SM3(X, 15) ^ shift_SM3(X, 23));
}

void SM3::SM3_hash() {
    sc_biguint<512> message;

    if(SM3_opt_in.read() == 1) {
        opt = 1;
    }

    if(rst_in.read() == false || SM3_rst_in.read() == 1) {
        message = 0;
        CF_count = 0;
        W64_count = 0;
        W68_count = 16;
        opt = 0;
        V_ready_out_count = 0;
        V_ready_out.write(V_ready_out_count);

        for(int j = 0; j < 68; j++) {
            W68[j] = 0;
        }

        for(int j = 0; j < 64; j++) {
            W64[j] = 0;
        }

        T_init();

        for(int j = 0; j < 8; j++) {
            V[j] = IV[j];
            V_buf[j] = IV[j];
        }

    }
    else if(rst_in.read() == true && opt == 1 && SM3_opt_in.read() == 0) {

        //message_ext
        message = message_in.read();

        for(int j = 0; j < 16; j++) {
            W68[j] = message.range(511-j*32, 480-j*32);
        }

#ifdef USING_SM3_AC
        if(W68_count>=16 && W68_count<=67) {
            W68[W68_count] = P1(W68[W68_count - 16] ^ W68[W68_count - 9] ^ (shift_SM3(W68[W68_count - 3], 15))) ^ (shift_SM3(W68[W68_count - 13], 7)) ^ W68[W68_count - 6];
            W68_count++;
        }

        if(W64_count>=0 && W64_count<=63) {
            W64[W64_count] = W68[W64_count] ^ W68[W64_count + 4];
            W64_count++;
        }

        //CF
        if(CF_count>=0 && CF_count<=63) {
            SS1 = shift_SM3((shift_SM3(V_buf[A], 12) + V_buf[E] + shift_SM3(T[CF_count], CF_count % 32)), 7);
            SS2 = SS1 ^ (shift_SM3(V_buf[A], 12));
            TT1 = FF(V_buf[A], V_buf[B], V_buf[C], CF_count) + V_buf[D] + SS2 + W64[CF_count];
            TT2 = GG(V_buf[E], V_buf[F], V_buf[G], CF_count) + V_buf[H] + SS1 + W68[CF_count];
            V_buf[D] = V_buf[C];
            V_buf[C] = shift_SM3(V_buf[B], 9);
            V_buf[B] = V_buf[A];
            V_buf[A] = TT1;
            V_buf[H] = V_buf[G];
            V_buf[G] = shift_SM3(V_buf[F], 19);
            V_buf[F] = V_buf[E];
            V_buf[E] = P0(TT2);

            CF_count++;
        }
        else {
            for(int j = 0; j < 8; j++) {
                V[j] = V_buf[j] ^ V[j];
                V_buf[j] = V[j];
            }

            V_finish = (V[0],V[1],V[2],V[3],V[4],V[5],V[6],V[7]);
            V_out.write(V_finish);
            V_ready_out_count++;
            V_ready_out.write(V_ready_out_count);
            CF_count = 0;
            W64_count = 0;
            W68_count = 16;
            opt = 0;
        }
#elif defined USING_SM3_AP
        for(W68_count=16; W68_count<=67; W68_count++) {
            W68[W68_count] = P1(W68[W68_count - 16] ^ W68[W68_count - 9] ^ (shift_SM3(W68[W68_count - 3], 15))) ^ (shift_SM3(W68[W68_count - 13], 7)) ^ W68[W68_count - 6];
        }

        for(W64_count=0; W64_count<=63; W64_count++) {
            W64[W64_count] = W68[W64_count] ^ W68[W64_count + 4];
        }

        //CF
        for(CF_count=0; CF_count<=63; CF_count++) {
            SS1 = shift_SM3((shift_SM3(V_buf[A], 12) + V_buf[E] + shift_SM3(T[CF_count], CF_count % 32)), 7);
            SS2 = SS1 ^ (shift_SM3(V_buf[A], 12));
            TT1 = FF(V_buf[A], V_buf[B], V_buf[C], CF_count) + V_buf[D] + SS2 + W64[CF_count];
            TT2 = GG(V_buf[E], V_buf[F], V_buf[G], CF_count) + V_buf[H] + SS1 + W68[CF_count];
            V_buf[D] = V_buf[C];
            V_buf[C] = shift_SM3(V_buf[B], 9);
            V_buf[B] = V_buf[A];
            V_buf[A] = TT1;
            V_buf[H] = V_buf[G];
            V_buf[G] = shift_SM3(V_buf[F], 19);
            V_buf[F] = V_buf[E];
            V_buf[E] = P0(TT2);

            if(CF_count == 63) {

                for(int j = 0; j < 8; j++) {
                    V[j] = V_buf[j] ^ V[j];
                    V_buf[j] = V[j];
                }

                V_finish = (V[0],V[1],V[2],V[3],V[4],V[5],V[6],V[7]);
                V_out.write(V_finish);
                V_ready_out_count++;
                V_ready_out.write(V_ready_out_count);
                opt = 0;
            }
        }
#elif defined USING_SM3_NO
        for(W68_count=16; W68_count<=67; W68_count++) {
            W68[W68_count] = P1(W68[W68_count - 16] ^ W68[W68_count - 9] ^ (shift_SM3(W68[W68_count - 3], 15))) ^ (shift_SM3(W68[W68_count - 13], 7)) ^ W68[W68_count - 6];
        }

        for(W64_count=0; W64_count<=63; W64_count++) {
            W64[W64_count] = W68[W64_count] ^ W68[W64_count + 4];
        }

        //CF
        for(CF_count=0; CF_count<=63; CF_count++) {
            SS1 = shift_SM3((shift_SM3(V_buf[A], 12) + V_buf[E] + shift_SM3(T[CF_count], CF_count % 32)), 7);
            SS2 = SS1 ^ (shift_SM3(V_buf[A], 12));
            TT1 = FF(V_buf[A], V_buf[B], V_buf[C], CF_count) + V_buf[D] + SS2 + W64[CF_count];
            TT2 = GG(V_buf[E], V_buf[F], V_buf[G], CF_count) + V_buf[H] + SS1 + W68[CF_count];
            V_buf[D] = V_buf[C];
            V_buf[C] = shift_SM3(V_buf[B], 9);
            V_buf[B] = V_buf[A];
            V_buf[A] = TT1;
            V_buf[H] = V_buf[G];
            V_buf[G] = shift_SM3(V_buf[F], 19);
            V_buf[F] = V_buf[E];
            V_buf[E] = P0(TT2);

            if(CF_count == 63) {

                for(int j = 0; j < 8; j++) {
                    V[j] = V_buf[j] ^ V[j];
                    V_buf[j] = V[j];
                }

                V_finish = (V[0],V[1],V[2],V[3],V[4],V[5],V[6],V[7]);
                V_out.write(V_finish);
                V_ready_out_count++;
                V_ready_out.write(V_ready_out_count);
                opt = 0;
            }
        }
#endif

    }
    else return;
}

