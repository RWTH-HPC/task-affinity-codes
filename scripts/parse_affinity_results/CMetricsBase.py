from abc import ABCMeta, abstractmethod
from CMeasurement import *

class CMetricsBase(metaclass=ABCMeta):

    @abstractmethod
    def load_metrics(self, path: str, meas: CMeasurement) -> bool:
        pass