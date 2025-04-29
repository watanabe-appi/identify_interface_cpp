import yaml
from generate_setup import get_parameters

def get_interface(filename, LX, LY, LZ, num_molecules, identification):
    with open(filename, "r") as f:
        lines = f.read().splitlines()
        idx = 0
        for i, l in enumerate(lines):
            if "ITEM: ATOMS id type xs ys zs" in l:
                type2 = []
                for j in range(num_molecules):
                    line = lines[i+j+1]
                    id, type, x, y, z = map(float, line.split())
                    x *= LX
                    y *= LY
                    z *= LZ
                    if type == 2:
                        type2.append([x, y, z])
                interface = []
                calc_interface(type2, interface, LX)
                output = f"{identification}/interface_{idx:03}.dat"
                save_file(interface, output)
                idx += 1



def get_num_of_molecules(filename):
     with open(filename, "r") as f:
        line = ""
        while True:
            if "ITEM: NUMBER OF ATOMS" in line:
                N = int(f.readline())
                return N
            else:
                line = f.readline()



def get_size(filename):
     with open(filename, "r") as f:
        line = ""
        while True:
            if "ITEM: BOX BOUNDS pp pp pp" in line:
                LX = float(f.readline().split()[1])
                LY = float(f.readline().split()[1])
                LZ = float(f.readline().split()[1])
                return LX, LY, LZ
            else:
                line = f.readline()


def calc_interface(input, output, LX):
        #num_bin = 32  #分割数
        #h = LX / (num_bin+1)  #分割幅
        h = 150 / 32
        num_bin = int((LX - h/2) // h)
        sum_y = []
        for i in range(num_bin):
            sum_y.append([0, 0])

        for x, y, z in input:
            for i in range(num_bin):
                if h * i <= x <= h * (i+1):
                    sum_y[i][0] += y
                    sum_y[i][1] += 1
        
        for i in range(num_bin):
            output.append([i*h + h/2, sum_y[i][0] / sum_y[i][1], sum_y[i][1]])

            

def save_file(input, output):
    with open(output, "w") as f:
        for x, y, n in input:
            f.write(f"{x} {y}\n")
    print(f"Generated {output}")



def main():
    rx, ry, rz, density, tempreture, A, num_waves, F, step = get_parameters()
    identification = f"rho{density}_{rx}x{ry}x{rz}_T{tempreture}_N{num_waves}_A{A}_F{F}"
    filename = f"{identification}/{identification}.lammpstrj"
    num_molecules = get_num_of_molecules(filename)
    LX, LY, LZ = get_size(filename)
    get_interface(filename, LX, LY, LZ, num_molecules, identification)


if __name__ == "__main__":
    main()

