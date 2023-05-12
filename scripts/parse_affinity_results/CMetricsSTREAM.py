from CMetricsBase import *

class CMetricsSTREAM(CMetricsBase):

    def load_metrics(self, path: str, meas: CMeasurement) -> bool:
        with open(path) as f: lines = [x.strip() for x in list(f)]
        for line in lines:
            if "Elapsed time for program" in line:
                spl = line.split("\t")
                meas.time_work = float(spl[1])
                return True
        return False