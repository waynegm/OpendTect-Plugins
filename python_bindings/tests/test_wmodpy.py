import pytest
import sys, platform, json

if platform.system() == 'Linux':
    sys.path.insert(0, "/opt/seismic/OpendTect_6/6.6.0/bin/lux64/Release")
elif platform.system() == 'Windows':
    sys.path.insert(0, "C:/Program Files/OpendTect/6.6.0/bin/win64/Release")

import wmodpy as wm
import pandas as pd


@pytest.fixture
def data_root():
    if platform.system() == 'Linux':
        return "/mnt/Data/seismic/ODData"
    elif platform.system() == 'Windows':
	    return "Y:/seismic/ODData"

@pytest.fixture
def data_root_simple():
    if platform.system() == 'Linux':
	    return "/mnt/Data/seismic/CooperBasin/ODData"
    elif platform.system() == 'Windows':
	    return "Y:/seismic/CooperBasin/ODData"

def test_get_surveys(data_root_simple):
    result = wm.get_surveys(data_root_simple)
    assert result == ["13CP06_Dundinna", "Cooper2D"]

def test_get_survey_info(data_root):
    result = wm.get_survey_info(data_root, ["F3_Demo_2020", "Blake_Ridge_Hydrates_3D", "USGS_Central_Alaska_v5"])
    assert result == {
                        'Name': ["F3 Demo 2020", "Blake_Ridge_Hydrates_3D", "USGS Central Alaska"],
                        'Type': ["2D3D", "3D", "2D"],
                        'crs': ["EPSG:23031", "EPSG:32618", ""]
                    }

def test_get_survey_info_df(data_root):
    result = wm.get_survey_info_df(data_root, ["F3_Demo_2020","Penobscot", "Blake_Ridge_Hydrates_3D", "USGS_Central_Alaska_v5"])
    testdict = {
                    'Name': ["F3 Demo 2020", "Penobscot", "Blake_Ridge_Hydrates_3D", "USGS Central Alaska"],
                    'Type': ["2D3D", "2D3D", "3D", "2D"],
                    'crs': ["EPSG:23031", "", "EPSG:32618", ""]
                }
    assert result.equals(pd.DataFrame(data=testdict))


def test_get_survey_features(data_root):
    result = json.loads(wm.get_survey_features(data_root, ["F3_Demo_2020"]))
    testgeodict = {
		    'type': 'FeatureCollection',
		    'features': [{
				    'type': 'Feature',
				    'properties': {"Name": "F3 Demo 2020", "Type": "2D3D", "crs": "EPSG:23031"},
				    'geometry': {"type": "Polygon", "coordinates": [[[4.644771779014873, 54.79611472015208],[5.0141138363413145, 54.79650898789612],[5.014324061777345, 54.94250714050207],[4.643644415587539, 54.942121470402164],[4.644771779014873, 54.79611472015208]]]}
				}]
		  }
    assert result == testgeodict

def test_Survey_class(data_root):
    f3demo = wm.Survey(data_root, "F3_Demo_2020")
    penobscot = wm.Survey(data_root, "Penobscot" )
    assert f3demo.name() == "F3 Demo 2020"
    assert penobscot.name() == "Penobscot"
    assert f3demo.epsg() == "EPSG:23031"
    assert penobscot.epsg() == ""
    assert f3demo.has2d() == True
    assert f3demo.has3d() == True
    test_dict = {
                    'Name': ["F3 Demo 2020"],
                    'Type': ["2D3D"],
                    'crs': ["EPSG:23031"]
                }
    assert f3demo.info() == test_dict
    assert f3demo.info_df().equals(pd.DataFrame(data=test_dict))
    assert f3demo.points(False) == [[605835.52047669, 6073556.38221996],[629576.2556206901, 6074219.892946],[629122.546655175, 6090463.1688065],[605381.811511175, 6089799.65808046],[605835.52047669, 6073556.38221996]]
    assert f3demo.points(True) == [[4.644771779014873, 54.79611472015208],[5.0141138363413145, 54.79650898789612],[5.014324061777345, 54.94250714050207],[4.643644415587539, 54.942121470402164],[4.644771779014873, 54.79611472015208]]
    assert f3demo.feature(True) == {'type': 'Feature', 'properties': {'Name': 'F3 Demo 2020', 'Type': '2D3D', 'crs': 'EPSG:23031'},
                                    'geometry': {'type': 'Polygon', 'coordinates': [[[4.644771779014873, 54.79611472015208],[5.0141138363413145, 54.79650898789612],[5.014324061777345, 54.94250714050207],[4.643644415587539, 54.942121470402164],[4.644771779014873, 54.79611472015208]]]}}

def test_Well_class(data_root):
    f3demo = wm.Survey(data_root, "F3_Demo_2020")
    penobscot = wm.Survey(data_root, "Penobscot" )
    f3wells = wm.Wells(f3demo)
    penwells = wm.Wells(penobscot)
    assert f3wells.names() == ['F02-1', 'F03-2', 'F03-4', 'F06-1']
    assert penwells.names() == ['L-30', 'B-41']
