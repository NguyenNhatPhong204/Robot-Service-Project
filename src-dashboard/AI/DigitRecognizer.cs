using Microsoft.ML.OnnxRuntime;
using Microsoft.ML.OnnxRuntime.Tensors;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;

namespace MockDashboard.AI
{
    public class DigitRecognizer
    {
        private InferenceSession _session;

        public DigitRecognizer(string modelPath)
        {
            if (!System.IO.File.Exists(modelPath))
                throw new Exception($"Model không tồn tại: {modelPath}");

            _session = new InferenceSession(modelPath);
            /*foreach (var inp in _session.InputMetadata)
            {
                var meta = inp.Value;
                var shape = string.Join(", ", meta.Dimensions);
                MessageBox.Show($"INPUT SHAPE = [{shape}]");
            }*/

        }

        public int Predict(float[] input)
        {
            // Tensor shape: [1, 1, 28, 28]
            var tensor = new DenseTensor<float>(input, new[] { 1, 28, 28, 1 });



            var inputs = new List<NamedOnnxValue>
            {
                NamedOnnxValue.CreateFromTensor("args_0", tensor)
            };

            // Không dùng "using var" để tránh lỗi C# 7.3
            IDisposableReadOnlyCollection<DisposableNamedOnnxValue> results = null;
            try
            {
                results = _session.Run(inputs);
                var output = results.First().AsEnumerable<float>().ToArray();
                int predicted = Array.IndexOf(output, output.Max());
                return predicted;
            }
            finally
            {
                if (results != null)
                    results.Dispose();
            }
        }
        public (int predicted, float confidence) PredictWithConfidence(float[] input)
        {
            // Tensor shape: [1, 28, 28, 1]
            var tensor = new DenseTensor<float>(input, new[] { 1, 28, 28, 1 });

            var inputs = new List<NamedOnnxValue>
    {
        NamedOnnxValue.CreateFromTensor("args_0", tensor)
    };

            IDisposableReadOnlyCollection<DisposableNamedOnnxValue> results = null;
            try
            {
                results = _session.Run(inputs);
                var output = results.First().AsEnumerable<float>().ToArray();

                int predicted = Array.IndexOf(output, output.Max());
                float confidence = output.Max(); 

                return (predicted, confidence);
            }
            finally
            {
                if (results != null)
                    results.Dispose();
            }
        }

    }
}
