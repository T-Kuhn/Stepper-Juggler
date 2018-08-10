using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace DataPointVisualization
{
    public struct DataPoint
    {
        public int VerticalPos;
        public int HorizontalPos;
        public int TimeStamp_ms;
    }

    // Importing dataPoints from a .log file.
    // .log file format:
    //     ...
    //     -9:164&19631
    //     -2:200&20007
    //     -17:200&20374
    //     ...
    //     VerticalPos:HorizontalPos&TimeStamp_ms
    public class DataPointImporter
    {
        private const string path = "rawDataPoints";
        private Queue<DataPoint> dataSet;

        public DataPointImporter()
        {
            // 1. load data from Resource-folder.
            var logFile = Resources.Load<TextAsset>(path);

            // 2. read data 1 line at a time.
            dataSet = new Queue<DataPoint>();
            var lines = logFile.text.Split('\n');
            foreach (var line in lines)
            {
                try
                {
                    var posAndTime = line.Split('&');
                    var pos = posAndTime[0];
                    var time_ms = posAndTime[1];
                    var posVerticalAndHorizontal = pos.Split(':');
                    // 3. parse and populate dataSet.
                    dataSet.Enqueue(new DataPoint() {
                        VerticalPos = System.Int32.Parse(posVerticalAndHorizontal[0]),
                        HorizontalPos = System.Int32.Parse(posVerticalAndHorizontal[1]),
                        TimeStamp_ms = System.Int32.Parse(time_ms)
                    });
                }
                catch (System.IndexOutOfRangeException e)
                {
                    Debug.Log("unusable line");
                }
            }
        }

        public DataPoint DequeueDataPoint()
        {
            return dataSet.Dequeue();
        }

        public DataPoint PeekDataPoint()
        {
            return dataSet.Peek();
        }

    }
}