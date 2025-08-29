-- CORCIONE RAFFAELLA - 10924817 - mat. 215144
-- PROVA FINALE DI RETI LOGICHE, a.a. 2024/25, POLITECNICO DI MILANO
-- INGEGNERIA INFORMATICA, prof. WILLIAM FORNACIARI

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity project_reti_logiche is -- COMPONENTE PRINCIPALE
    port(
        i_clk : in std_logic;
        i_rst : in std_logic;
        i_start : in std_logic;
        i_add: in std_logic_vector(15 downto 0);
        o_done: out std_logic;
        o_mem_addr : out std_logic_vector(15 downto 0);
        i_mem_data : in std_logic_vector(7 downto 0);
        o_mem_data : out std_logic_vector(7 downto 0);
        o_mem_we : out std_logic;
        o_mem_en : out std_logic
    );
end entity;

architecture project_reti_logiche_arch of project_reti_logiche is -- collegamento di tutti i componenti che realizzano l'architettura
    component fsm is
        port(
            start : in std_logic;
            i_clk : in std_logic;
            rst : in std_logic;       
            mem_en : out std_logic;
            mem_we : out std_logic;
            done : out std_logic;
            
            s0 : in std_logic;
            k : in std_logic_vector(15 downto 0);
            k_en : out std_logic;
            k_h : out std_logic;
            s0_en : out std_logic;
            c_en : out std_logic;
            w_en : out std_logic;
            w_sel : out std_logic;
            init_r : out std_logic;
            init_w : out std_logic;
            inc_8 : out std_logic;
            en_1r : out std_logic;
            en_1w : out std_logic;
            en_8 : out std_logic
        );
    end component;
    
    component shift_reg_16 is
        port ( 
            input : in std_logic_vector(7 downto 0);
            en : in std_logic;
            h : in std_logic;
            output : out std_logic_vector(15 downto 0);
            rst : in std_logic;
            i_clk : in std_logic
        );
    end component;
    
    component shift_reg_1 is
        port(
            input : in std_logic_vector(7 downto 0);
            enable : in std_logic;
            output : out std_logic;
            i_clk, rst : in std_logic
        );
    end component;
    
    component array_7_par_reg_8 is
        port(
            c_enable : in std_logic;
            c_in : in std_logic_vector(7 downto 0);
            i_clk, c_rst : in std_logic;
            c_101 : out std_logic_vector(7 downto 0);
            c_110 : out std_logic_vector(7 downto 0);
            c_111 : out std_logic_vector(7 downto 0);
            c_000 : out std_logic_vector(7 downto 0);
            c_001 : out std_logic_vector(7 downto 0);
            c_010 : out std_logic_vector(7 downto 0);
            c_011 : out std_logic_vector(7 downto 0)
        );
    end component;
    
    component mux_8 is
        port(
            input : in std_logic_vector(7 downto 0);
            sel : in std_logic;
            output : out std_logic_vector(7 downto 0)
        );
    end component;
    
    component addr_calc is
        port(
            add : in std_logic_vector(15 downto 0);
            k : in std_logic_vector(15 downto 0);
            init_read, init_write, inc_8, en_1r, en_8, en_1w : in std_logic;
            i_clk : in std_logic;
            o_mem_we : in std_logic;
            o_mem_addr : out std_logic_vector(15 downto 0)
        );
    end component;
    
    component filter is
        port(
            c_101 : in std_logic_vector(7 downto 0);
            c_110 : in std_logic_vector(7 downto 0);
            c_111 : in std_logic_vector(7 downto 0);
            c_000 : in std_logic_vector(7 downto 0);
            c_001 : in std_logic_vector(7 downto 0);
            c_010 : in std_logic_vector(7 downto 0);
            c_011 : in std_logic_vector(7 downto 0);
            w_101 : in std_logic_vector(7 downto 0);
            w_110 : in std_logic_vector(7 downto 0);
            w_111 : in std_logic_vector(7 downto 0);
            w_000 : in std_logic_vector(7 downto 0);
            w_001 : in std_logic_vector(7 downto 0);
            w_010 : in std_logic_vector(7 downto 0);
            w_011 : in std_logic_vector(7 downto 0);
            i_clk : in std_logic;
            s0 : in std_logic;
            result : out std_logic_vector(7 downto 0) 
        );
    end component;
    
    signal s0_internal, k_en_internal, k_h_internal, s0_en_internal, c_en_internal, w_en_internal, w_sel_internal, init_r_internal, init_w_internal, inc_8_internal, mem_we_internal, mem_en_internal, en_1r_internal, en_1w_internal, en_8_internal : std_logic;
    signal k_internal : std_logic_vector(15 downto 0);
    signal w_in_internal : std_logic_vector(7 downto 0);
    signal c_101_internal, c_110_internal, c_111_internal, c_000_internal, c_001_internal, c_010_internal, c_011_internal : std_logic_vector(7 downto 0);
    signal w_101_internal, w_110_internal, w_111_internal, w_000_internal, w_001_internal, w_010_internal, w_011_internal : std_logic_vector(7 downto 0);
begin
    controller : fsm port map(
        start => i_start,
        i_clk => i_clk,
        rst => i_rst,
        mem_en => mem_en_internal,
        mem_we => mem_we_internal,
        done => o_done,
        s0 => s0_internal,
        k => k_internal,
        k_en => k_en_internal,
        k_h => k_h_internal,
        s0_en => s0_en_internal,
        c_en => c_en_internal,
        w_en => w_en_internal,
        w_sel => w_sel_internal,
        init_r => init_r_internal,
        init_w => init_w_internal,
        inc_8 => inc_8_internal,
        en_1r => en_1r_internal,
        en_1w => en_1w_internal,
        en_8 => en_8_internal
    );
    
    k_reg : shift_reg_16 port map(
        i_clk=> i_clk,
        input => i_mem_data,
        en => k_en_internal,
        h => k_h_internal,
        rst => i_rst,
        output => k_internal
    );
    
    s0_reg : shift_reg_1 port map(
        i_clk => i_clk,
        rst => i_rst,
        input => i_mem_data,
        enable => s0_en_internal,
        output => s0_internal
    );
    
    c_reg : array_7_par_reg_8 port map(
        c_enable => c_en_internal,
        c_in => i_mem_data,
        i_clk => i_clk,
        c_rst => i_rst,
        c_101 => c_101_internal,
        c_110 => c_110_internal,
        c_111 => c_111_internal,
        c_000 => c_000_internal,
        c_001 => c_001_internal,
        c_010 => c_010_internal,
        c_011 => c_011_internal
    );
    
    w_reg : array_7_par_reg_8 port map(
        c_enable => w_en_internal,
        c_in => w_in_internal,
        i_clk => i_clk,
        c_rst => i_rst,
        c_101 => w_101_internal,
        c_110 => w_110_internal,
        c_111 => w_111_internal,
        c_000 => w_000_internal,
        c_001 => w_001_internal,
        c_010 => w_010_internal,
        c_011 => w_011_internal
    );
    
    w_mux : mux_8 port map(
        input => i_mem_data,
        sel => w_sel_internal,
        output => w_in_internal
    );
    
    mem_addr_calc : addr_calc port map(
        add => i_add,
        k => k_internal,
        i_clk => i_clk,
        init_read => init_r_internal,
        init_write => init_w_internal,
        inc_8 => inc_8_internal,
        o_mem_we => mem_we_internal,
        o_mem_addr => o_mem_addr,
        en_1r => en_1r_internal,
        en_1w => en_1w_internal,
        en_8 => en_8_internal
    );
    
    f : filter port map(
        c_101 => c_101_internal,
        c_110 => c_110_internal,
        c_111 => c_111_internal,
        c_000 => c_000_internal,
        c_001 => c_001_internal,
        c_010 => c_010_internal,
        c_011 => c_011_internal,
        w_101 => w_101_internal,
        w_110 => w_110_internal,
        w_111 => w_111_internal,
        w_000 => w_000_internal,
        w_001 => w_001_internal,
        w_010 => w_010_internal,
        w_011 => w_011_internal,
        i_clk => i_clk,
        s0 => s0_internal,
        result => o_mem_data
    );
    
    o_mem_we <= mem_we_internal;
    o_mem_en <= mem_en_internal;
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity fsm is -- MACCHINA A STATI FINITI
    port(
        start : in std_logic;
        i_clk : in std_logic;
        rst : in std_logic;       
        mem_en : out std_logic;
        mem_we : out std_logic;
        done : out std_logic;
        
        s0 : in std_logic;
        k : in std_logic_vector(15 downto 0);
        k_en : out std_logic;
        k_h : out std_logic;
        s0_en : out std_logic;
        c_en : out std_logic;
        w_en : out std_logic;
        w_sel : out std_logic;
        en_1r : out std_logic;
        en_1w : out std_logic;
        en_8 : out std_logic;
        init_r : out std_logic;
        init_w : out std_logic;
        inc_8 : out std_logic
    );
end entity;

architecture fsm_arch of fsm is
    type S is (WAIT_START, INIT, S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, FP, P1, P2, P3, P4, P4_bis, P5, P6, P7, P8, P9, FINAL);
    signal curr_state : S;
    signal counter : std_logic_vector (3 downto 0); -- per contare le iterazioni nello stesso stato
    signal counter_k : std_logic_vector(15 downto 0); -- per contare le k ripetizioni del ciclo di calcolo del filtro
begin 
    delta : process (i_clk, rst) -- funzione degli stati
    begin
        if rst='1' then
            curr_state <= WAIT_START;
        elsif i_clk'event and i_clk='1' then
            case curr_state is
            when WAIT_START =>
                if (start = '1' and rst = '0') then
                    curr_state <= INIT;
                else 
                    curr_state <= WAIT_START;
                end if;
            when INIT =>
                counter <= (others => '0');
                counter_k <= (others => '0');
                curr_state <= S1;
            when S1 =>
                curr_state <= S2;
            when S2 =>
                curr_state <= S3;
            when S3 =>
                curr_state <= S4;
            when S4 =>
                curr_state <= S5;
            when S5 =>
                curr_state <= S6;
            when S6 =>
                curr_state <= S7;
            when S7 =>
                curr_state <= S8;
            when S8 =>
                if counter = "1000" then
                    curr_state <= S9;
                else 
                    counter <= std_logic_vector(unsigned(counter) + 1);
                    curr_state <= S8;
                end if;
            when S9 =>
                counter <= (others => '0');
                curr_state <= S10;
            when S10 =>
                curr_state <= FP;
            when FP =>
                curr_state <= P1;
            when P1 =>
                curr_state <= P2;
            when P2 =>
                curr_state <= P3;
            when P3 =>
                curr_state <= P4;
            when P4 =>
                curr_state <= P4_bis;
                counter_k <= std_logic_vector(unsigned(counter_k) + 1);
            when P4_bis =>
                curr_state <= P5;
            when P5 =>
                if counter = "0100" then
                    curr_state <= P6;
                else
                    counter <= std_logic_vector(unsigned(counter) + 1);
                    curr_state <= P5;
                end if;
            when P6 =>
                counter <= (others => '0');
                curr_state <= P7;
            when P7 =>
                if to_integer(unsigned(counter_k)) <= to_integer(unsigned(k))-4 then
                    curr_state <= P4;
                elsif to_integer(unsigned(counter_k)) = to_integer(unsigned(k)) then
                    curr_state <= FINAL;
                else
                    curr_state <= P8;
                end if;
            when P8 =>
                counter_k <= std_logic_vector(unsigned(counter_k) + 1);
                curr_state <= P9;
            when P9 =>
                curr_state <= P5;
            when others =>
                if start = '0' then
                    curr_state <= WAIT_START;
                else
                    curr_state <= FINAL;
                end if;
            end case;
        end if;
    end process;
    
    lambda : process (curr_state, counter, s0) -- funzione delle uscite
    begin
        done <= '0';
        mem_en <= '0';
        mem_we <= '0';
        k_en <= '0';
        k_h <= '0';
        s0_en <= '0';
        c_en <= '0';
        w_en <= '0';
        w_sel <= '0';
        init_r <= '0';
        init_w <= '0';
        en_1r <= '0';
        en_1w <= '0';
        inc_8 <= '0';
        en_8 <= '0';
        
        -- manca il caso curr_state = WAIT_START perché in quello stato le uscite sono tutte nulle
        if curr_state = INIT then
            init_r <= '1';
            init_w <= '1';
        elsif curr_state = S1 then
            init_r <= '1';
            init_w <= '1';
            en_1r <= '1';
            mem_en <= '1';
        elsif curr_state = S2 then
            init_w <= '1';
            en_1r <= '1';
            k_en <= '1';
            mem_en <= '1';
        elsif curr_state = S3 then
            init_w <= '1';
            k_en <= '1';
            k_h <= '1';
            mem_en <= '1';
        elsif curr_state = S4 then
            init_w <= '1';
            s0_en <= '1';
            mem_en <= '1';
        elsif curr_state = S5 then
            init_w <= '1';
            mem_en <= '1';
        elsif curr_state = S6 then
            init_w <= '1';
            if s0 = '0' then
                en_1r <= '1';
            else
                inc_8 <= '1';
                en_8 <= '1';
            end if;
            mem_en <= '1';
        elsif curr_state <= S7 then
            init_w <= '1';
            if S0 = '1' then
                inc_8 <= '1';
            end if;
            mem_en <= '1';
        elsif curr_state = S8 then
            init_w <= '1';
            mem_en <= '1';
            if counter = "0000" then
                en_1r <= '1';
            elsif (counter = "0110" or counter = "0111") then
                c_en <= '1';
            elsif (counter = "0001" or counter = "0010" or counter = "0011" or counter = "0100" or counter = "0101") then
                en_1r <= '1';
                c_en <= '1';
            end if;
        elsif curr_state = S9 then
            init_w <= '1';
            mem_en <= '1';
            if s0 = '0' then
                inc_8 <= '1';
                en_8 <= '1';
           else
                en_1r <= '1';
           end if;
        elsif curr_state = S10 then
            if S0 = '0' then
                inc_8 <= '1';
            end if;
        elsif curr_state = FP then
            mem_en <= '1';
            w_en <= '1';
            en_1r <= '1';
        elsif curr_state = P1 then
            mem_en <= '1';
            w_en <= '1';
            w_sel <= '1';
            en_1r <= '1';
        elsif curr_state = P2 then
            mem_en <= '1';
            w_en <= '1';
            w_sel <= '1';
            en_1r <= '1';
        elsif curr_state = P3 then
            mem_en <= '1';
            w_en <= '1';
            w_sel <= '1';
            en_1r <= '1';
        elsif curr_state = P4 then
            mem_en <= '1';
            w_en <= '1';
            w_sel <= '1';
        elsif curr_state = P4_bis then
            mem_en <= '1';
            w_sel <= '1';
        elsif curr_state = P5 then
            mem_en <= '1';
        elsif curr_state = P6 then
            mem_en <= '1';
            mem_we <= '1';
        elsif curr_state = P7 then
            mem_en <= '1';
            en_1r <= '1';
            en_1w <= '1';
        elsif curr_state = P8 then
            mem_en <= '1';
            w_en <= '1';
        elsif curr_state = P9 then
            mem_en <= '1';
        elsif curr_state = FINAL then
            done <= '1';
        end if;          
    end process;
end architecture;

library ieee;
use ieee.std_logic_1164.all;

entity shift_reg_16 is -- REGISTRO DA 16 BIT
    port ( 
        input : in std_logic_vector(7 downto 0);
        en : in std_logic;
        h : in std_logic; -- indica se su input si trova la prima o la seconda meta' del valore da salvare
        output : out std_logic_vector(15 downto 0);
        rst : in std_logic;
        i_clk : in std_logic
    );
end shift_reg_16;

architecture shift_reg_16_arch of shift_reg_16 is
signal sel1 : std_logic;
signal sel2 : std_logic;
begin
    sel1 <= en and (not h);
    sel2 <= en and h;
    
    out_p : process (i_clk, rst) is
    begin
        if (rst = '1') then
            output <= "0000000000000000";
        elsif (i_clk'event and i_clk='1') then
            if sel1 = '1' then
                output(15 downto 8) <= input;
            end if;
            if sel2 = '1' then
                output(7 downto 0) <= input;
            end if;
        end if;
    end process; 
end architecture;

library ieee;
use ieee.std_logic_1164.all;

entity shift_reg_1 is -- REGISTRO DA 1 BIT
    port(
        input : in std_logic_vector(7 downto 0);
        enable : in std_logic;
        output : out std_logic;
        i_clk, rst : in std_logic
    );
end entity;

architecture shift_reg_1_arch of shift_reg_1 is
begin
    out_p : process(i_clk, rst)
    begin
        if (rst = '1') then
            output <= '0';
        elsif (i_clk'event and i_clk='1') then
            if enable = '1' then
                output <= input(0);
            end if;
        end if;
    end process;
end architecture;

library ieee;
use ieee.std_logic_1164.all;

entity array_7_par_reg_8 is -- ARRAY A SCORRIMENTO DI 7 REGISTRI DA 8 BIT
    port(
        c_enable, i_clk, c_rst : in std_logic;
        c_in : in std_logic_vector(7 downto 0);
        c_101, c_110, c_111, c_000, c_001, c_010, c_011 : out std_logic_vector(7 downto 0)
    );
end entity;

architecture array_7_par_reg_8_arch of array_7_par_reg_8 is
    component par_reg_8 is
        port(
            input : in std_logic_vector(7 downto 0);
            enable, i_clk, rst : in std_logic;
            output : out std_logic_vector(7 downto 0)
        );
    end component;
    
    signal c_101_internal, c_110_internal, c_111_internal, c_000_internal, c_001_internal, c_010_internal, c_011_internal : std_logic_vector(7 downto 0);
begin
    reg101 : par_reg_8 port map(
        input => c_in,
        enable => c_enable,
        i_clk => i_clk,
        rst => c_rst,
        output => c_101_internal
    );
    reg110 : par_reg_8 port map(
        input => c_101_internal,
        enable => c_enable,
        i_clk => i_clk,
        rst => c_rst,
        output => c_110_internal
    );
    reg111 : par_reg_8 port map(
        input => c_110_internal,
        enable => c_enable,
        i_clk => i_clk,
        rst => c_rst,
        output => c_111_internal
    );
    reg000 : par_reg_8 port map(
        input => c_111_internal,
        enable => c_enable,
        i_clk => i_clk,
        rst => c_rst,
        output => c_000_internal
    );
    reg001 : par_reg_8 port map(
        input => c_000_internal,
        enable => c_enable,
        i_clk => i_clk,
        rst => c_rst,
        output => c_001_internal
    );
    reg010 : par_reg_8 port map(
        input => c_001_internal,
        enable => c_enable,
        i_clk => i_clk,
        rst => c_rst,
        output => c_010_internal
    );
    reg011 : par_reg_8 port map(
        input => c_010_internal,
        enable => c_enable,
        i_clk => i_clk,
        rst => c_rst,
        output => c_011_internal
    );
    
    c_101 <= c_101_internal;
    c_110 <= c_110_internal;
    c_111 <= c_111_internal;
    c_000 <= c_000_internal;
    c_001 <= c_001_internal;
    c_010 <= c_010_internal;
    c_011 <= c_011_internal;
end architecture;

library ieee;
use ieee.std_logic_1164.all;

entity par_reg_8 is -- SINGOLO REGISTRO DA 8 BIT
    port(
        input : in std_logic_vector(7 downto 0);
        enable, i_clk, rst : in std_logic;
        output : out std_logic_vector(7 downto 0)
    );
end entity;

architecture par_reg_8_arch of par_reg_8 is
begin
    out_p : process(i_clk, rst)
    begin
        if(rst = '1') then
            output <= "00000000";
        elsif(i_clk'event and i_clk='1') then
            if enable = '1' then
                output <= input;
            end if;
        end if;
    end process;
end architecture;

library ieee;
use ieee.std_logic_1164.all;

entity mux_8 is -- MULTIPLEXER ASINCRONO CON INGRESSO E USCITA DA 8 BIT, SULL'INGRESSO RELATIVO A sel='0' PONE IL VALORE 0
    port(
        input : in std_logic_vector(7 downto 0);
        sel : in std_logic;
        output : out std_logic_vector(7 downto 0)
    );
end entity;

architecture mux_8_arch of mux_8 is
begin
    o_gen : for index in 7 downto 0 generate
        output(index) <= ('0' and not sel) or (input(index) and sel);
    end generate;
end architecture;

library ieee;
use ieee.std_logic_1164.all;

entity addr_calc is -- COMPONENTE CHE EFFETTUA IL CALCOLO E LA MEMORIZZAZIONE DEGLI INDIRIZZI DI LETTURA E SCRITTURA IN MEMORIA
    port(
        add : in std_logic_vector(15 downto 0);
        k : in std_logic_vector(15 downto 0);
        init_read, init_write, inc_8, en_1r, en_1w, en_8 : in std_logic;
        i_clk : in std_logic;
        o_mem_we : in std_logic;
        o_mem_addr : out std_logic_vector(15 downto 0)
    );
end entity;

architecture addr_calc_arch of addr_calc is
    component adder_1 is
        port(
            input : in std_logic_vector(15 downto 0);
            i_clk : in std_logic;
            en : in std_logic;
            result : out std_logic_vector(15 downto 0)
        );
    end component;
    component adder_8 is
        port(
            input : in std_logic_vector(15 downto 0);
            i_clk : in std_logic;
            en : in std_logic;
            result : out std_logic_vector(15 downto 0)
        );
    end component;
    component adder_17 is
        port(
            input_add : in std_logic_vector(15 downto 0);
            input_k : in std_logic_vector(15 downto 0);
            i_clk : in std_logic;
            result : out std_logic_vector(15 downto 0)
        );
    end component;
    component mux_16 is
        port(
            in1 : in std_logic_vector(15 downto 0);
            in2 : in std_logic_vector(15 downto 0);
            sel : in std_logic;
            output : out std_logic_vector(15 downto 0)
        );
    end component;
    
    signal addr_read, addr_write, x, y, z, w, u : std_logic_vector(15 downto 0);
begin
    incr_read : adder_1 port map(
        i_clk => i_clk,
        en => en_1r,
        input => addr_read,
        result => x
    );
    incr_write : adder_1 port map(
        i_clk => i_clk,
        en => en_1w,
        input => addr_write,
        result => u
    );
    sum_8_read : adder_8 port map(
        i_clk => i_clk,
        input => addr_read,
        en => en_8,
        result => y
    );
    sum_k_17 : adder_17 port map(
        i_clk => i_clk,
        input_add => add,
        input_k => k,
        result => w
    );
    mux_1 : mux_16 port map(
        in1 => x,
        in2 => y,
        sel => inc_8,
        output => z
    );
    mux_2 : mux_16 port map(
        in1 => z,
        in2 => add,
        sel => init_read,
        output => addr_read
    );
    mux_3 : mux_16 port map(
        in1 => u,
        in2 => w,
        sel => init_write,
        output => addr_write
    );
    mux_4 : mux_16 port map(
        in1 => addr_read,
        in2 => addr_write,
        sel => o_mem_we,
        output => o_mem_addr
    );
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity adder_1 is -- SOMMATORE SINCRONO AL CLOCK CHE INCREMENTADI 1 L'INDIRIZZO PER LA MEMORIA FORNITO IN INGRESSO
    port(
        input : in std_logic_vector(15 downto 0);
        i_clk, en : in std_logic;
        result : out std_logic_vector(15 downto 0)
    );
end entity;

architecture adder_1_arch of adder_1 is
begin
    out_p : process(i_clk)
    begin
        if(i_clk'event and i_clk='1') then
            case en is
                when '1' =>
                    result <= std_logic_vector(unsigned(input) + 1);
                when others =>
                    result <= input;
            end case;
        end if;
    end process;
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity adder_8 is -- COMPONENTE CHE INCREMENTA DI 8 L'INDIRIZZO PER LA MEMORIA FORNITO IN INGRESSO
    port(
        input : in std_logic_vector(15 downto 0);
        i_clk, en : in std_logic;
        result : out std_logic_vector(15 downto 0)
    );
end entity;

architecture adder_8_arch of adder_8 is
begin
    out_p : process(i_clk)
    begin
        if(i_clk'event and i_clk='1') then
            case en is
                when '1' =>
                    result <= std_logic_vector(unsigned(input) + 8);
                when others =>
                    result <= input;
            end case;
        end if;
    end process;
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity adder_17 is -- COMPONENTE CHE CALCOLA IL VALORE DELL'INDIRIZZO ADD+17+K
    port(
        input_add, input_k : in std_logic_vector(15 downto 0);
        i_clk : in std_logic;
        result : out std_logic_vector(15 downto 0)
    );
end entity;

architecture adder_17_arch of adder_17 is
begin
    out_p : process(i_clk)
    begin
        if(i_clk'event and i_clk='1') then
            result <= std_logic_vector(unsigned(input_add) + unsigned(input_k) + 17);
        end if;
    end process;
end architecture;

library ieee;
use ieee.std_logic_1164.all;

entity mux_16 is -- MULTIPLEXER ASINCRONO CON INGRESSI E USCITE A 16 BIT
    port(
        in1, in2 : in std_logic_vector(15 downto 0);
        sel : in std_logic;
        output : out std_logic_vector(15 downto 0)
    );
end entity;

architecture mux_16_arch of mux_16 is
begin
    o_gen : for index in 15 downto 0 generate
        output(index) <= (in1(index) and not sel) or (in2(index) and sel);
    end generate;
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity filter is -- COMPONENTE CHE, PRESI IN INGRESSO 7 COEFFICIENTI E 7 VALORI DI W, CALCOLA IL VALORE DEL FILTRO RIFERITO A w_000
    port(
        c_101, c_110, c_111, c_000, c_001, c_010, c_011 : in std_logic_vector(7 downto 0);
        w_101, w_110, w_111, w_000, w_001, w_010, w_011 : in std_logic_vector(7 downto 0);
        i_clk : in std_logic;
        s0 : in std_logic;
        result : out std_logic_vector(7 downto 0) 
    );
end entity;

architecture filter_arch of filter is
    component multiplier is
        port(
            in0, in1 : in std_logic_vector(7 downto 0);
            prod : out std_logic_vector(15 downto 0);
            en : in std_logic;
            i_clk : in std_logic
        );
    end component;
    component big_adder is
        port(
            in0, in1, in2, in3, in4, in5, in6 : in std_logic_vector(15 downto 0);
            sum : out std_logic_vector(19 downto 0);
            i_clk : in std_logic
        );
    end component;
    component norm_12 is
        port(
            w_in : in std_logic_vector(19 downto 0);
            w_out : out std_logic_vector(19 downto 0);
            i_clk : in std_logic
        );
    end component;
    component norm_60 is
            port(
                w_in : in std_logic_vector(19 downto 0);
                w_out : out std_logic_vector(19 downto 0);
                i_clk : in std_logic
            );
    end component;
    component mux_20 is
        port(
            in1 : in std_logic_vector(19 downto 0);
            in2 : in std_logic_vector(19 downto 0);
            sel : in std_logic;
            output : out std_logic_vector(19 downto 0)
        );
    end component;
    component comp is
        port(
            input : in std_logic_vector(19 downto 0);
            i_clk : in std_logic;
            output : out std_logic_vector(7 downto 0)
        );
    end component;

    signal mul_0, mul_1, mul_2, mul_3, mul_4, mul_5, mul_6 : std_logic_vector(15 downto 0);
    signal add, norm_12_out, norm_60_out, norm_val : std_logic_vector(19 downto 0);
begin
    mul0 : multiplier port map(
        in0 => c_101,
        in1 => w_101,
        i_clk => i_clk,
        en => s0,
        prod => mul_0
    );
    mul1 : multiplier port map(
        in0 => c_110,
        in1 => w_110,
        i_clk => i_clk,
        en => '1',
        prod => mul_1
    );
    mul2 : multiplier port map(
        in0 => c_111,
        in1 => w_111,
        i_clk => i_clk,
        en => '1',
        prod => mul_2
    );
    mul3 : multiplier port map(
        in0 => c_000,
        in1 => w_000,
        i_clk => i_clk,
        en => '1',
        prod => mul_3
    );
    mul4 : multiplier port map(
        in0 => c_001,
        in1 => w_001,
        i_clk => i_clk,
        en => '1',
        prod => mul_4
    );
    mul5 : multiplier port map(
        in0 => c_010,
        in1 => w_010,
        i_clk => i_clk,
        en => '1',
        prod => mul_5
    );
    mul6 : multiplier port map(
        in0 => c_011,
        in1 => w_011,
        i_clk => i_clk,
        en => s0,
        prod => mul_6
    );
    adder : big_adder port map(
        in0 => mul_0,
        in1 => mul_1,
        in2 => mul_2,
        in3 => mul_3,
        in4 => mul_4,
        in5 => mul_5,
        in6 => mul_6,
        i_clk => i_clk,
        sum => add
    );
    norm_12_c : norm_12 port map(
        w_in => add,
        w_out => norm_12_out,
        i_clk => i_clk
    );
    norm_60_c : norm_60 port map(
        w_in => add,
        w_out => norm_60_out,
        i_clk => i_clk
    );
    degree_mux : mux_20 port map(
        in1 => norm_12_out,
        in2 => norm_60_out,
        sel => s0,
        output => norm_val
    );
    comp_cut : comp port map(
        i_clk => i_clk,
        input => norm_val,
        output => result
    );
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity multiplier is -- COMPONENTE CHE, DATI DUE INGRESSI A 8 BIT, RESTITUISCE IL VALORE DEL LORO PRODOTTO SU 16 BIT.
    port(
        in0, in1 : in std_logic_vector(7 downto 0);
        prod : out std_logic_vector(15 downto 0);
        en : in std_logic;
        i_clk : in std_logic
    );
end entity;

architecture multiplier_arch of multiplier is
signal in0_internal, in1_internal : signed(7 downto 0);
begin 
    in0_internal <= resize(signed(in0), 8);
    in1_internal <= resize(signed(in1), 8);
    
    out_p : process(i_clk)
    begin
         if(i_clk'event and i_clk='1') then
            if en = '1' then
                prod <= std_logic_vector(in0_internal * in1_internal);
            else 
                prod <= (others => '0');
            end if;
         end if;
    end process;
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity big_adder is -- COMPONENTE CHE EFFETTUA LA SOMMA SU 20 BIT DI 7 NUMERI DA 16 BIT
    port(
        in0, in1, in2, in3, in4, in5, in6 : in std_logic_vector(15 downto 0);
        sum : out std_logic_vector(19 downto 0);
        i_clk : in std_logic
    );
end entity;

architecture big_adder_arch of big_adder is
signal in0_internal, in1_internal, in2_internal, in3_internal, in4_internal, in5_internal, in6_internal : signed(19 downto 0);
begin 
    in0_internal <= resize(signed(in0), 20);
    in1_internal <= resize(signed(in1), 20);
    in2_internal <= resize(signed(in2), 20);
    in3_internal <= resize(signed(in3), 20);
    in4_internal <= resize(signed(in4), 20);
    in5_internal <= resize(signed(in5), 20);
    in6_internal <= resize(signed(in6), 20);
    
    out_p : process(i_clk)
    begin
         if(i_clk'event and i_clk='1') then
            sum <= std_logic_vector(in0_internal + in1_internal + in2_internal + in3_internal + in4_internal + in5_internal + in6_internal);
         end if;
    end process;
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity norm_12 is -- COMPONENTE CHE EFFETTUA LA DIVISIONE APPROSSIMATA PER 12
    port(
        w_in : in std_logic_vector(19 downto 0);
        w_out : out std_logic_vector(19 downto 0);
        i_clk : in std_logic
    );
end entity;

architecture norm_12_arch of norm_12 is
    component shift_4 is
        port(
            input : in std_logic_vector(19 downto 0);
            output : out std_logic_vector(19 downto 0);
            i_clk : in std_logic
        );
    end component;
    component shift_6 is
        port(
            input : in std_logic_vector(19 downto 0);
            output : out std_logic_vector(19 downto 0);
            i_clk : in std_logic
        );
    end component;
    component shift_8 is
        port(
            input : in std_logic_vector(19 downto 0);
            output : out std_logic_vector(19 downto 0);
            i_clk : in std_logic
        );
    end component;
    component shift_10 is
        port(
            input : in std_logic_vector(19 downto 0);
            output : out std_logic_vector(19 downto 0);
            i_clk : in std_logic
        );
    end component;
    component mux_20 is
        port(
            in1 : in std_logic_vector(19 downto 0);
            in2 : in std_logic_vector(19 downto 0);
            sel : in std_logic;
            output : out std_logic_vector(19 downto 0)
        );
    end component;
    component adder_20_5 is
        port(
            in0, in1, in2, in3, in4 : in std_logic_vector(19 downto 0);
            sum : out std_logic_vector(19 downto 0);
            i_clk : in std_logic
        );
    end component;
    
    signal out_4, out_6, out_8, out_10, out_fix : std_logic_vector(19 downto 0);
begin
    sh4 : shift_4 port map(
        input => w_in,
        output => out_4,
        i_clk => i_clk
    );
    sh6 : shift_6 port map(
        input => w_in,
        output => out_6,
        i_clk => i_clk
    );
    sh8 : shift_8 port map(
        input => w_in,
        output => out_8,
        i_clk => i_clk
    );
    sh10 : shift_10 port map(
        input => w_in,
        output => out_10,
        i_clk => i_clk
    );
    fix : mux_20 port map(
        in1 => "00000000000000000000",
        in2 => "00000000000000000100",
        sel => w_in(19),
        output => out_fix
    );
    adder : adder_20_5 port map(
        in0 => out_4,
        in1 => out_6,
        in2 => out_8,
        in3 => out_10,
        in4 => out_fix,
        sum => w_out,
        i_clk => i_clk
    );
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity norm_60 is -- COMPONENTE CHE EFFETTUA LA DIVISIONE APPROSSIMATA PER 60
    port(
        w_in : in std_logic_vector(19 downto 0);
        w_out : out std_logic_vector(19 downto 0);
        i_clk : in std_logic
    );
end entity;

architecture norm_60_arch of norm_60 is
    component shift_6 is
        port(
            input : in std_logic_vector(19 downto 0);
            output : out std_logic_vector(19 downto 0);
            i_clk : in std_logic
        );
    end component;
    component shift_10 is
        port(
            input : in std_logic_vector(19 downto 0);
            output : out std_logic_vector(19 downto 0);
            i_clk : in std_logic
        );
    end component;
    component mux_20 is
        port(
            in1 : in std_logic_vector(19 downto 0);
            in2 : in std_logic_vector(19 downto 0);
            sel : in std_logic;
            output : out std_logic_vector(19 downto 0)
        );
    end component;
    component adder_20_5 is
        port(
            in0, in1, in2, in3, in4 : in std_logic_vector(19 downto 0);
            sum : out std_logic_vector(19 downto 0);
            i_clk : in std_logic
        );
    end component;
    
    signal out_6, out_10, out_fix : std_logic_vector(19 downto 0);
begin
    sh6 : shift_6 port map(
        input => w_in,
        output => out_6,
        i_clk => i_clk
    );
    sh10 : shift_10 port map(
        input => w_in,
        output => out_10,
        i_clk => i_clk
    );
    fix : mux_20 port map(
        in1 => "00000000000000000000",
        in2 => "00000000000000000010",
        sel => w_in(19),
        output => out_fix
    );
    adder : adder_20_5 port map(
        in0 => out_6,
        in1 => out_10,
        in2 => out_fix,
        in3 => std_logic_vector(to_signed(0, 20)),
        in4 => std_logic_vector(to_signed(0, 20)),
        sum => w_out,
        i_clk => i_clk
    );
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity shift_4 is -- COMPONENTE CHE EFFETTUA LO SHIFT LOGICO A DESTRA DI 4 BIT
    port(
        input : in std_logic_vector(19 downto 0);
        output : out std_logic_vector(19 downto 0);
        i_clk : in std_logic
    );
end entity;

architecture shift_4_arch of shift_4 is
begin
    out_p : process(i_clk)
    begin
        if(i_clk'event and i_clk='1') then
            output <= std_logic_vector(shift_right(signed(input), 4));
        end if;
    end process;
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity shift_6 is -- COMPONENTE CHE EFFETTUA LO SHIFT LOGICO A DESTRA DI 6 BIT
    port(
        input : in std_logic_vector(19 downto 0);
        output : out std_logic_vector(19 downto 0);
        i_clk : in std_logic
    );
end entity;

architecture shift_6_arch of shift_6 is
begin
    out_p : process(i_clk)
    begin
        if(i_clk'event and i_clk='1') then
            output <= std_logic_vector(shift_right(signed(input), 6));
        end if;
    end process;
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity shift_8 is -- COMPONENTE CHE EFFETTUA LO SHIFT LOGICO A DESTRA DI 8 BIT
    port(
        input : in std_logic_vector(19 downto 0);
        output : out std_logic_vector(19 downto 0);
        i_clk : in std_logic
    );
end entity;

architecture shift_8_arch of shift_8 is
begin
    out_p : process(i_clk)
    begin
        if(i_clk'event and i_clk='1') then
            output <= std_logic_vector(shift_right(signed(input), 8));
        end if;
    end process;
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity shift_10 is -- COMPONENTE CHE EFFETTUA LO SHIFT LOGICO A DESTRA DI 10 BIT
    port(
        input : in std_logic_vector(19 downto 0);
        output : out std_logic_vector(19 downto 0);
        i_clk : in std_logic
    );
end entity;

architecture shift_10_arch of shift_10 is
begin
    out_p : process(i_clk)
    begin
        if(i_clk'event and i_clk='1') then
            output <= std_logic_vector(shift_right(signed(input), 10));
        end if;
    end process;
end architecture;

library ieee;
use ieee.std_logic_1164.all;

entity mux_20 is -- MULTIPLEXER ASINCRONO CON INGRESSI E USCITA A 20 BIT
    port(
        in1 : in std_logic_vector(19 downto 0);
        in2 : in std_logic_vector(19 downto 0);
        sel : in std_logic;
        output : out std_logic_vector(19 downto 0)
    );
end entity;

architecture mux_20_arch of mux_20 is
begin
    o_gen : for index in 19 downto 0 generate
        output(index) <= (in1(index) and not sel) or (in2(index) and sel);
    end generate;
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity adder_20_5 is -- COMPONENTE CHE SOMMA, SU 20 BIT, 5 NUMERI RAPPRESENTATI SU 20 BIT
-- componente utilizzato sia nella normalizzazione con n=12 sia con n=60, in quest'ultimo caso, dato che la somma è di soli tre valori,
-- le due porte in3 e in4 sono collegate a un segnale costante pari a 0.
    port(
        in0, in1, in2, in3, in4 : in std_logic_vector(19 downto 0);
        sum : out std_logic_vector(19 downto 0);
        i_clk : in std_logic
    );
end entity;

architecture adder_20_5_arch of adder_20_5 is
signal in0_internal, in1_internal, in2_internal, in3_internal, in4_internal : signed(19 downto 0);
begin  
    in0_internal <= signed(in0);
    in1_internal <= signed(in1);
    in2_internal <= signed(in2);
    in3_internal <= signed(in3);
    in4_internal <= signed(in4);
    
    out_p : process(i_clk)
    begin
         if(i_clk'event and i_clk='1') then
            sum <= std_logic_vector(in0_internal + in1_internal + in2_internal + in3_internal + in4_internal);
         end if;
    end process;
end architecture;

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity comp is -- COMPONENTE CHE RIPORTA L'INGRESSO AD UN VALORE CHE RAPPRESENTA UN NUMERO COMPRESO TRA -128 E 127, SU 8 BIT
    port(
        input : in std_logic_vector(19 downto 0);
        i_clk : in std_logic;
        output : out std_logic_vector(7 downto 0)
    );
end entity;

architecture comp_arch of comp is
begin
    out_p : process(i_clk)
    begin
        if rising_edge(i_clk) then
            if to_integer(signed(input)) <= -128 then
                output <= "10000000";
            elsif to_integer(signed(input)) >= 127 then
                output <= "01111111";
            else
                output <= input(7 downto 0);
            end if;
        end if;
    end process;
end architecture;