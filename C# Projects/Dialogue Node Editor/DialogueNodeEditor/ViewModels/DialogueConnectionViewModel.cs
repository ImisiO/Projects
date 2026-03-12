using DialogueNodeEditor.Models;
using System;
using System.Windows;
using System.Windows.Media;

namespace DialogueNodeEditor.ViewModels
{
    public class DialogueConnectionViewModel : ViewModelBase
    {
        #region Init / Deinit

        /// <summary>
        /// Constructor for DialogueConnectionViewModel object
        /// </summary>
        /// <param name="model">Dialogue connection model</param>
        public DialogueConnectionViewModel(DialogueConnection model)
        {
            Model = model;
        }

        #endregion // Init / Deinit

        #region Member Variables

        /// <summary>Dialogue connection model</summary>
        public DialogueConnection Model { get; }
        
        /// <summary>The id of the connection</summary>
        public Guid Id => Model.Id;
        
        /// <summary>The id of the node the connection is starting from</summary>
        public Guid FromId => Model.FromId;

        /// <summary>The id of the node that the connection is going to</summary>
        public Guid ToId => Model.ToId;

        /// <summary>[STORE] Path geometry connecting the two nodes</summary>
        private PathGeometry _geometry = new();
        /// <summary>Path geometry connecting the two nodes</summary>
        public PathGeometry Geometry
        {
            get => _geometry;
            private set => SetField(ref _geometry, value);
        }

        #endregion // Member Variables

        #region Helper Functions

        /// <summary>
        /// Generates path geometry between the passed 'from' point to the passed 'to' point
        /// </summary>
        /// <param name="from">Point to generate path geometry from</param>
        /// <param name="to">Point to generate path geometry to</param>
        public void Recalculate(Point from, Point to)
        {
            double cp = Math.Abs(to.X - from.X) * 0.55 + 60;

            PathFigure figure = new PathFigure { StartPoint = from, IsFilled = false };
            figure.Segments.Add(new BezierSegment(
                new Point(from.X + cp, from.Y),
                new Point(to.X - cp, to.Y),
                to,
                isStroked: true));

            PathGeometry geo = new PathGeometry();
            geo.Figures.Add(figure);

            Geometry = geo;
        }

        #endregion // Helper Functions
    }
}
