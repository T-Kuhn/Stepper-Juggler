using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace DataPointVisualization
{
    public class SpotSpawner : MonoBehaviour
    {
        [SerializeField] GameObject spotPrefab;
        private DataPointImporter dataPointImporter;

        void Start()
        {
            //dataSet.Enqueue(new DataPoint() { VerticalPos = -55, HorizontalPos = -163, TimeStamp_ms = 276352 });
            dataPointImporter = new DataPointImporter();
            Debug.Log(dataPointImporter);
        }

        void Update()
        {
            var dataPoint = dataPointImporter.PeekDataPoint();
            var timeSinceStart_ms = (int)(Time.time * 1000);

            if (dataPoint.TimeStamp_ms <= timeSinceStart_ms)
            {
                var _ = dataPointImporter.DequeueDataPoint();

                var v = dataPoint.VerticalPos / 200f;
                var h = dataPoint.HorizontalPos / 200f;

                var spot = Instantiate(spotPrefab, new Vector3(h, 0f, v), Quaternion.identity);
            }
        }
    }
}
