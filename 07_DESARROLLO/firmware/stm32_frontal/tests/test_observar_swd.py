"""Compatibilidad de capturas SWD, sin conectar a ninguna placa."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('observer', Path(__file__).resolve().parents[1]/'tools/observar_swd.py')
observer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(observer)

class SnapshotTests(unittest.TestCase):
    def test_legacy(self):
        raw = bytearray(96)
        struct.pack_into('<I', raw, 0, 123456)
        raw[78:86] = bytes(range(8))
        decoded = observer.decodificar_ui(raw)
        self.assertEqual(decoded['tick'], 123456)
        self.assertEqual(decoded['bms355'], '0001020304050607')
        self.assertNotIn('pedal_free', decoded)

    def test_dashboard(self):
        raw = bytes(96)+struct.pack('<2I10B2x', 20, 0xffffffff, 1, 0, 1, 0, 0, 1, 1, 0, 3, 1)
        result = observer.decodificar_ui(raw)
        self.assertEqual(result['front_left_age'], 20)
        self.assertEqual(result['front_right_age'], 0xffffffff)
        self.assertEqual(result['drive_input'], 1)
        self.assertEqual(result['relay_state'], 3)
        self.assertEqual(result['backend_gear'], 1)
        self.assertEqual(result['pedal_free'], 0)

    def test_unknown_layout(self):
        for size in (0, 94, 100, 114, 120):
            with self.assertRaises(ValueError): observer.decodificar_ui(bytes(size))

if __name__ == '__main__': unittest.main()
