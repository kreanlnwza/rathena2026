#!/usr/bin/env python3
"""Negative fixtures for validation failures that dictionaries can hide."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from validate_manifest import missing_status_reference_errors, unique_index


class ValidationFailureTests(unittest.TestCase):
    def test_duplicate_id_is_rejected(self) -> None:
        records = [
            {"id": 6608, "name": "DK_SERVANT_W_CLEAVE"},
            {"id": 6608, "name": "IG_SHIELD_SLAM"},
        ]
        with self.assertRaisesRegex(ValueError, "duplicate ID 6608"):
            unique_index(records, "id", "name", "fixture")

    def test_duplicate_name_is_rejected(self) -> None:
        records = [
            {"id": 6608, "name": "DK_SERVANT_W_CLEAVE"},
            {"id": 6609, "name": "DK_SERVANT_W_CLEAVE"},
        ]
        with self.assertRaisesRegex(ValueError, "duplicate name DK_SERVANT_W_CLEAVE"):
            unique_index(records, "id", "name", "fixture")

    def test_missing_output_and_required_statuses_are_reported(self) -> None:
        records = [{
            "Id": 6608,
            "Name": "DK_SERVANT_W_CLEAVE",
            "Status": "Missing_Output",
            "Requires": {"Status": {"Known": True, "Missing_Requirement": True}},
        }]
        errors = missing_status_reference_errors(records, {"Known"})
        self.assertEqual(2, len(errors))
        self.assertTrue(any("Missing_Output" in error for error in errors))
        self.assertTrue(any("Missing_Requirement" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
