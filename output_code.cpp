                                                        template<typename/**/T/*clvkikuthsgfdfvb*//*jtljduzwfdljhcjk*/                                           
                                                      ,const/**/int/**/B/*rotm*//*aeqctbdevksvttuv*//*pllsquhlzzlolata*/                                         
                                                    >class/**/bitset_chan{/*zuxper*//*jlgspzbhynzptwaj*//*eagonkakzbrzbgex*/                                     
                                                    public:using/**/T_T/*ycsfwqpvkyr*//*mumahesfozszalpt*//*dzzsptwrjtgmvcsx*/                                   
                                                  =T;static_assert/*kqqmahonmznxuqdn*//*nvmctsmhacjjzhvw*//*hgyqmnoezzpjemje*/                                   
                                                (sizeof(T/*eal*//*tolywxdldfabsdki*//*dvrysmhnvacuerzp*//*isnkirzlkqmhutqh*/                                     
                                                )*8==/**//*fqoxvmaaumqid*//*buawwtsayhjregqz*//*brassggykmlvyvmd*//*hfdhfeixhd  bxid*/                           
                                                B,"check block width")/**//*wmtjhicrhwowst*//*dqxkolfreaxhblst*//*pgcnmmxfaplrnyyi*/                             
                                              ;static_assert(std/*pztlnm*//*xoagtconlxhuenad*//*cgvxluzgdjlvoiru*//*mhkfujbummnskmxk*/                           
                                              ::is_same<T/*pzmwyxarnimmq*//*zlfkogggjkvqahse*//*szloamaearyckxti*//*qdmibqrwfpyphjaw*/                           
                                            ,uint64_t>::/*asxlbypwcehr*//*alkpjqagkzyvmsol*//*pzxdbcnpvxglevsu*//*uqxcxkwvfegroouv*/                             
                                            value,"modify popcnt(), ctz(), clz()")/*igvdoi*//*jlylmsakgrqnamrw*//*wtuxixvlojmkbtpy*/                             
                                          ;public:static/*lcglgbylqvcn*//*xmprxvzfjabjdhrn*//*sgfywbszelwrrbxp*//*avkmpdzammehsdbr*/                             
                                          inline/**/constexpr/**/bool/**/on/*cjmczvclusnry*//*gvdogyjdldfyrfgl*//*zitzcklxlfdtnrwc*/                             
                                          (int/*rm*/i,/*eksmlodzcyimnbaw*//*vcyzrnpjfbcxuxcq*//*izykkasmibadcwrx*//*lvboaqtcetewqgaz*/                           
                                        T/*h*/x)noexcept/*oeopqtnrmutopvgc*//*hlshlrxnaxhetcwy*//*ynsjzuzeqsmorsdd*//*hjmvaymbbnbsajkr*/                         
                                        {return((/*odfrj*//*cehsnsjuzrtydfhf*//*kpqaxwaescerneig*//*zpixbysaekowvjto*//*zyvqlnexsgxabybs*/                       
                                      T(1)/*nwgyhsvvqougwi*//*fyfrsgwvghbyzplw*//*nwtdplomgsbmmwdr*//*msbaoywlkbqcutpr*//*yrdzndbtocnjwiyt*/                     
                                      <<i)&/*vezjxkxik*//*faoqofzpcfyqwpqc*//*lnltamfwedwcgqnb*//*tipacorydiqfupss*//*yqqkaaxiopfhmmtc*/                         
                                      x)!=0/*qnpkxpl*//*lbdczzyuntmessge*//*uvsvopdqdbscssbq*//*yyzpgodmwcvrnpmb*//*ehbjnkaxfufoquha*/                           
                                      ;}static/**/inline/*hwvmxzdktrwmfefy*//*bxyorgaagzcqibyv*//*djuxocomhzljzoxq*//*yqwfwkleqkhlgzed*/                         
                                    constexpr/**/T/**/prefix(/*uwmsanpwlja*//*zmeekqoeibdclerz*//*uzzquoswvlnpplnw*//*tmfpmkgumsfnmxdc*/                         
                                    int/*qtu*/i)noexcept/*kswjlfvoxnwrmetk*//*uaokfjykjeyvyxsb*//*ezktcnfsjr  gsji*//*acbygdijdhzwbxph*/                         
                                    {return(i/*fsrutow*//*hkwbihaeymeyyhfx*//*ywiomhpniqsttisn*//*fqwvvvpj    okhw*//*hmzhkxalqlxqixqk*/                         
                                    >=B)?/*zewyydeuo*//*muuofwzvidjjkhjz*//*gufxnzfanllzcmhe*//*tkbtirufyu      */    /*xgxqhdftuqvjmk*/                         
                                  ~T(0/*ccdspuyvufdj*//*isvvuvtfhxatorib*//*rjdyepxuxsycubvj*//*ddbibsfsys      */    /*cytaaybxqfgtbx*/                         
                                  ):((/*xwew*//*pwacxfnoxwhnzyti*//*zandjuqnmllfbsxi*//*bawvctqyjijzbkgt*/            /*dgclevqierqdoq*/                         
                                  T(1)/*xz*//*avxeffnnajvrbwni*//*wfzfrzgvtbfkyyfy*//*eakjpfxxysurcrkt*/              /*bkjkkctjdzkxej*/                         
                                  <<i)-/*e*//*foodbqlcdhhiozeb*//*mplcnsfpajizgweo*//*qtazkuzwdzlndahg*/              /*mfwtvrqshewjllgt*/                       
                                  T(1)/*td*//*zyuzbyouvckhksos*//*rxrrsrhvknmwgfmp*//*xzswlhflmkqypkrq*/                /*uhzddvzxgyoqyt*/                       
                                  );}static/**//*ycrcnsbqgbrkj*//*ehgkahxhcwsxzemp*//*jyytzpcoqfbqgd  */              /*wheikopdyhghiwck*/                       
                                  inline/**/constexpr/**/T/**/suffix/*wnwxqe*//*tmjjgcwvuvlaroox*/                      /*ouffojjfkiuasc*/                       
                                (int/**/i)/*nbcllnanckfg*//*agxfoohawipsdafb*//*evgbimvibvjcipkh*/                      /*ck  rrvfibycdc*/                       
                                noexcept{return~/*apdtkknifgclrysz*//*jzidknzwchejvdwp*//*bxpwmwppcunjsnyt*/          /*fy  xprdjqnidebb*/                       
                                prefix(B-/*das*//*lildhnwrqutqcgcm*//*snfhpvaecrjivsnd*//*bheixphnpclwcved*/          /*  quqa  glwiiche*/                       
                                i);}/*jdnwhyonbyun*//*nqtqmbeszzpxwatc*//*auvyzxivvitzjkpd*//*vpukqczcfsfiogcj*/      /*  vrtteemzywfnxw*/                       
                              static/**/inline/**/constexpr/**/T/*dqjynj*//*mjyfkourlmczwkvq*//*mzbwepgtionewhke*/    /*awmptbczsckejgmv*/                       
                            range(int/**/l/*qmsijg*//*bnanwidw  fomsjt*//*zdtjiyna        *//*ebxvjhlpxnknedhp*/        /*skfixnejvvsrpuov*/                     
                            ,int/*kr*/r)/*azyksiytbzhrjdht*/        /*        *//*              mn*/  /*xkutuz      gj*//*mukomytupuizqzro*/                     
                            noexcept{return/**/prefix/*leq*/                                                        /**//*ujeoyjqjzeydibya*/                     
                          (r/*igbbhq*//*mfwjibxhxvoosgil*/                          )^                              /**/      /*hxjykpwbpfvj*/                   
                          prefix(/*s*//*scdjmlcpautolylw*/                          l-                            /**/            /*hdysctba*/                   
                        1)/*qdzqrbcw*//*drnpkzmemiaqgpqx*/                          ;}                            /**/              /*fgtjbsbz*/                 
                        static/**/constexpr/**/int/**/popcnt                        /**/                        /*dkvg*/      /*fqrvxqephrvfnnou*/               
                        (T/**/x)/*ua*//*xyjmiphncuftdxdt*/                        /*kr*/                    /*lupoah    */  /*dnlhcpiascoggety*/                 
                      noexcept{return/**//*csglfkegvqclhej*/                      /**/                      /*              */__builtin_popcountll               
                      (x);/*wgkwbqcxls*//*kbwenbmopsmddkdb*/                                                /*lykcbs      */    /*an      femfqrte*/             
                    }static/**/constexpr/**/int/*mfjobtgxweh*/                                              /*kghsvwau        *//*          vicjze*/             
                    clz(T/**/x/*ynndbnbm*//*kygiewmrxqwgqslb*/                                                    /*foxzkueuezrc*/            /*xmho*/           
                  )noexcept{return/*cjvpqoskkzeyplxt*//*iqlkhdtj        */                            /*tl*/  /*dxvzcxyybxkatfyw*/            /*omkm*/           
                  __builtin_clzll(x)/*swohjv*//*qhnaojqzhuyeqxzd*/          /*  rk  fr*//*wm          mkyp*//*tesnod    glfrtl*/                /*mxyd*/         
                ;}static        constexpr/*vaa*//*emcanryoktcorvda*/                                /*korj*//*mzohezdghvfnkyvm*/                /*adnw*/         
              int/*c*/          ctz(T/*esaibsegb*//*ipgleehfvpaqarod*/                            /*nnfr*/  /*tj  smllcdarlk  */                /*bvvw*/         
              x)/**/              noexcept{/*ulnwr*//*gyijqqrytyzrwckd*/                      /*iyll*/            /*avyciapj  */                  /*su*/         
            return                    __builtin_ctzll(x/**//*jtgzadlgcfcgt*/              /*ov*/                  /*    dv  br*/                  /*sc*/         
          )/*nlh*/                          ;}static/*ilxzrrlsiidsbnig*//*ezit        pxlh*/                      /*  trtbpo*/                    /*sq*/         
          inline                            constexpr/**/int/**/block_id/*rxqpcrfqmkolselv*/                      /*dgrl*/                        /*vw*/         
        (/*z*/                            int/*b*/i)/*ijvbybkxvoqkfenw*//*xomirdbflaoexuqd*/                      /*aj*/                        /*xboz*/         
        /*dc*/                            noexcept{return/**/i/*daoh*//*dqnwirenuubkbuom*/                  /*sarc*/                              /*rp*/         
      /B/**/                              ;}/*tl*//*jzaotk  rhwdbmqq*//*zhfdnywrmiihkttl*/                /*zceu*/                              /*hflc*/         
      public                                :inline/**/T/*    gqxnci*//*jchhzwzzmvdczwke*/              /*fimk*/                                  /*yw*/         
    submask(                              int/**/l/*kzauhmozoxgqoczw*//*qkhfbejvmvinzbte*/      /*  uykirrnzqp*/                                  /*bi*/         
    ,/*x*/                                int/**/r)/*tigznwaojusyv*//*fqlvwsmwvyizgbjc*/    /*ghfavhikrfzxhbkk*/                                  /*by*/         
  /*gsus*/                                const/**/noexcept{int/*bbf*//*tpeqdjcdpzewwves*/  /*txwhftfkuhuolgjf*/                  /*            *//*lz*/         
  /*xium*/                              bx  =block_id(/*enbfvmegvpnq*//*zrcecermxyjuzsgw*/  /*ejlraywxuxjwkjui*/                /**//*            jeon*/         
  l/*o*/                      );  assert/*  siqwte*//*phmmpjplarvfaxlp*//*xztxswezypvhjjth*//*apjpcwzpbbdpvihh*/                /**//*            hash*/         
(bx/*s*/                      ==block_id/*    oq*//*suoyudqwdigaxhng*//*pzhxkvxikfonaaxa*//*          wkhhxv*/                  /**/            /*uupv*/         
(r/*sk*/                      ))/*gwfpedjz    wmxt*//*zwwduatnxpfqadvn*//*desqbgywgrwphsio*/                    /*            vnez*/            /*tonj*/         
;return(                      b/**//*hydkq    cqle*//*wlrmrrszhcbeznyn*//*puisvdnezvhzzaqs*/                                  /**/              /*ystc*/         
[/*gnh*/                      bx/*chfjjfzgdhevljkq*//*iyhywznsuydzlngn*//*kunyknwbndrqlaye*/                                  ]&                /*xtxq*/         
/*vkso*/                      range(/*grtfvkftct  *//*eekmkizgsgpuawcy*//*eccxnsqrgztjorwd*/                      l-                            /*ozqn*/         
bx*B+/*a          */            /*vswxxwlu    */      /*obfbuqyaokxgzd*//*tfdycqgjhbignptc*/                      /*            */              /*vuwo*/         
1,r-/**/                        /*yu          */        /*bfehzgulzzip*//*bswtflecpimhpgdv*/                                                    /*zhuk*/         
bx*B/**/                    +/*ekrmr          */          /*fehrugmyxj*//*firvdximzzahitsp*/                        /*            */            /*kycx*/         
1));/**/                    /*napd            */            /*eecdoxrj*//*ksveqbjgeghopsxr*/                                  /*ni*/          /*mkasoh*/         
}inline void trim()noexcept{b.back()&=prefix(n%B==0?B:n%B);}public:int n,m;std::vector<T>b;bitset_chan(int n):bitset_chan(n,false){};bitset_chan(int n,
bool init):n(n),m((n+B-1)/B),b(m,init?~T(0):T(0)){trim();};inline void set(int i,bool val)noexcept{assert(0<=i and i<n);if(val)b[i/B]|=(T(1)<<(i%B));else b[i
/B]&=~(T(1)<<(i%B));}inline bool get(int i)const noexcept{assert(0<=i and i<n);return(b[i/B]&(T(1)<<(i%B)))!=0;}void reset()noexcept{std::fill(b.begin(),
b.end(),T(0));}void operator&=(const bitset_chan&other){for(int i=0;i<std::min(m,other.m);i++)b[i]&=other.b[i];if(m>other.m)std::fill(b.begin()+other.m,b.
begin()+m,T(0));}void operator|=(const bitset_chan&other){for(int i=0;i<std::min(m,other.m);i++)b[i]|=other.b[i];trim();}void operator^=(const bitset_chan&
other){for(int i=0;i<std::min(m,other.m);i++)b[i]^=other.b[i];trim();}void operator<<=(int x){if(x==0)return;if(x>=n){reset();return;}const int s=x/B,d=x%B
,r=B-d;if(d>0){for(int i=m-1-s;i>0;i--)b[i+s]=(b[i]<<d)|(b[i-1]>>r);b[s]=b[0]<<d;}else{for(int i=m-1-s;i>0;i--)b[i+s]=b[i];b[s]=b[0];}std::fill(b.begin()
,b.begin()+s,T(0));trim();}void operator>>=(int x){if(x==0)return;if(x>=n){reset();return;}const int s=x/B,d=x%B,l=B-d;if(d>0){for(int i=s;i<m-1;i++)b[i
-s]=(b[i]>>d)|(b[i+1]<<l);b[m-1-s]=b[m-1]>>d;}else for(int i=s;i<m;i++)b[i-s]=b[i];std::fill(b.begin()+m-s,b.end(),T(0));}bool operator==(const bitset_chan&
other){return((n==other.n)and b==other.b);}bool operator!=(const bitset_chan&other){return!(*this==other);}bitset_chan operator&(const bitset_chan&other){
bitset_chan result(*this);result&=other;return result;}bitset_chan operator|(const bitset_chan&other){bitset_chan result(*this);result|=other;return result;
}bitset_chan operator^(const bitset_chan&other){bitset_chan result(*this);result^=other;return result;}bitset_chan operator>>(int x){bitset_chan result(*this
);result>>=x;return result;}bitset_chan operator<<(int x){bitset_chan result(*this);result<<=x;return result;}bitset_chan operator~(){bitset_chan result(*this);
for(auto&v:result)v=~v;result.trim();return result;}int count()const noexcept{return std::accumulate(b.begin(),b.end(),0,[](int sum,T value){return sum+popcnt
(value);});}int find_first(){int pos=-1;for(int bi=0;bi<m;bi++){if(b[bi]==T(0))continue;pos=ctz(b[bi])+bi*B;break;}return pos;}int find_last(){int pos=-1;for(
int bi=m-1;bi>=0;bi--){if(b[bi]==T(0))continue;pos=B-clz(b[bi])-1+bi*B;break;}return pos;}void range_process(int l,int r,auto block_brute,auto block_quick){
assert(0<=l and l<=r and r<n);int bl=block_id(l),br=block_id(r);if(bl==br)block_brute(l,r);else{block_brute(l,(bl+1)*B-1);for(int bi=bl+1;bi<br;bi++)
block_quick(bi);block_brute(br*B,r);}}void range_set(int l,int r,bool val){auto block_brute=[&](int l,int r)->void{int bi=block_id(l);T mask=range(l-bi*B+1,r-bi
*B+1);if(val)b[bi]|=mask;else b[bi]&=~mask;};auto block_quick=[&](int bi)->void{b[bi]=(val?~T(0):T(0));};range_process(l,r,block_brute,block_quick);}int count(
int l,int r){int cnt=0;auto block_brute=[&](int l,int r)->void{cnt+=popcnt(submask(l,r));};auto block_quick=[&](int bi)->void{cnt+=popcnt(b[bi]);};
range_process(l,r,block_brute,block_quick);return cnt;}int find_first(int l,int r){int pos=-1;auto block_brute=[&](int l,int r)->void{for(int i=l;i<=r and
pos==-1;i++)if(get(i))pos=i;};auto block_quick=[&](int bi)->void{if(b[bi]==T(0)or pos!=-1)return;pos=ctz(b[bi])+bi*B;};range_process(l,r,block_brute,
block_quick);return pos;}int find_last(int l,int r){int pos=-1;auto block_brute=[&](int l,int r)->void{for(int i=l;i<=r;i++)if(get(i))pos=i;};auto block_quick=[
&](int bi)->void{if(b[bi]==T(0))return;pos=B-clz(b[bi])-1+bi*B;};range_process(l,r,block_brute,block_quick);return pos;}friend std::ostream&operator<<(std::
ostream&os,const bitset_chan&bitset){for(int i=bitset.m-1;i>=0;--i)os<<std::bitset<B>(bitset.b[i]);os<<'\n';return os;}};using bitset_chan64=bitset_chan<
uint64_t,bit_width(uint64_t())>;
