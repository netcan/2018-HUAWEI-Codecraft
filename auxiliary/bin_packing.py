

class SF:
    def __init__(self, cpu, mem):
        self.cpu = cpu
        self.mem = mem

    def __repr__(self):
        return "<CPU: {} MEM: {} M/C: {}>".format(self.cpu, self.mem, self.mem_cpu_ratio())

    def mem_cpu_ratio(self):
        return self.mem / self.cpu


class Server(SF):
    def __init__(self, cpu, mem):
        super().__init__(cpu, mem)
        self.remain_cpu = cpu
        self.remain_mem = mem

    def __repr__(self):
        assert self.remain_mem >= 0 and self.remain_cpu >= 0
        return "<CPU: {}/{} MEM: {}/{} M/C: {}>".format(self.cpu - self.remain_cpu, self.cpu,
                                                        self.mem - self.remain_mem, self.mem,
                                                        self.mem_cpu_ratio())

    def mem_usage_ratio(self):
        return (self.mem - self.remain_mem) / self.mem

    def cpu_usage_ratio(self):
        return (self.cpu - self.remain_cpu) / self.cpu

    def usage_ratio(self):
        return (self.mem_usage_ratio() + self.cpu_usage_ratio()) / 2

    def __isub__(self, flavor):
        assert isinstance(flavor, Flavor)
        self.remain_cpu -= flavor.cpu
        self.remain_mem -= flavor.mem
        return self


    pass


class Flavor(SF):
    def __mul__(self, other):
        cpu = self.cpu * other
        mem = self.mem * other
        return Flavor(cpu, mem)


def load_info():
    servers = {}
    flavors = {}
    with open('flavors_info.txt', 'r') as f:
        lines = f.readlines()
        server_type_num = int(lines[0])
        servers = {
            l.split()[0].split('-')[0]: Server(int(l.split()[1]), int(l.split()[2]))
            for l in lines[1:server_type_num + 1]
        }

        flavor_type_num = int(lines[2 + server_type_num])
        flavors = {
            l.split()[0]: Flavor(int(l.split()[1]), int(l.split()[2]) / 1024)
            for l in lines[3 + server_type_num:3 + server_type_num + flavor_type_num]
        }
        print(flavor_type_num)

    return servers, flavors


def checking():
    servers, flavors = load_info()
    with open('全420.txt', 'r') as f:
        lines = f.readlines()
        # General
        for idx, idxn in zip([21, 95, 516], [93, 514, 875]):
            for _srv in lines[idx:idxn]:
                srv = _srv.split()
                print(srv[0])
                server = servers[srv[0].split('-')[0]]
                server = Server(server.cpu, server.mem)
                for i in range(1, len(srv), 2):
                    flavor_name, flavor_num = srv[i:i+2]
                    flavor_num = int(flavor_num)
                    print(flavors[flavor_name], flavor_num)
                    server -= flavors[flavor_name] * flavor_num

                print(server)
                print(server.usage_ratio())


if __name__ == '__main__':
    checking()

