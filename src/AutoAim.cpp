#include "Defines.h"

//#define CAMERA_DEBUG
//#define GUI
//#define USE_USER_BUFFERS
#define PRESELECT
#define MAX_DEVICES 64
#define MAX_FORMATS 64
#define MAX_PROPERTIES 64
#define TEMPLATE_FILE "template_240.jpg"
#define PREFERRED_RESOLUTION 240

#include <unicap.h>
#include <unicapgtk.h>
#include <ucil.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <fftw3.h>
#include <boost/thread.hpp>
#include <limits>
#include <Includes.h>

#ifdef GUI
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixdata.h>
#else
#include <jpeglib.h>
#endif

#include <NetServer.h>
#include <NetBuffer.h>
#include <MessageType.h>

using namespace std;
using namespace Network;

#ifdef GUI
GtkWidget* window;
GtkWidget* window2;
GtkWidget* video;
GtkWidget* image;
GtkWidget* image2;
#endif
GdkPixbuf* tpl_pix;
GdkPixbuf* tpl_pix2;
unsigned char* tpl;
unsigned char* tpl2;
fftw_complex *in_img, *out_img, *in_tpl, *out_tpl, *out_fft, *out;
fftw_plan plan_fwd_img;
fftw_plan plan_fwd_tpl;
fftw_plan plan_bwd;
clock_t start;
long long frameCount;
double frameRate;
NetServer* ns;
//ucil_video_file_object_t* vobj;

#ifdef CAMERA_DEBUG
struct widgets {
	GtkWidget *video_display;
	GtkWidget *format_selection;
	GtkWidget *property_dialog;
};
#endif

void set_format(unicap_handle_t handle) {
	unicap_format_t formats[MAX_FORMATS];
	int format_count;
	unicap_status_t status = STATUS_SUCCESS;
	int f = -1;

	for (format_count = 0; SUCCESS (status) && (format_count < MAX_FORMATS);
			format_count++) {
		status = unicap_enumerate_formats(handle, NULL, &formats[format_count], // (1)
				format_count);
		if (SUCCESS (status)) {
			printf("%d: %s\n", format_count, formats[format_count].identifier);
		} else {
			break;
		}
	}

	if (format_count == 0) {
		// no video formats
		return;
	}

	while ((f < 0) || (f >= format_count)) {
		printf("Use Format: ");
#ifdef PRESELECT
		f = 0;
		printf("%d\n", f);
#else
		scanf("%d", &f);
#endif
	}
	int to_select;
	if (formats[f].size_count) {
		// (2)
		int i;
		int s = -1;

		for (i = 0; i < formats[f].size_count; i++) {
			printf("%d: %dx%d\n", i, formats[f].sizes[i].width,
					formats[f].sizes[i].height);
			if (formats[f].sizes[i].height == PREFERRED_RESOLUTION)
				to_select = i;
		}

		while ((s < 0) || (s >= formats[f].size_count)) {
			printf("Select Size: ");
#ifdef PRESELECT
			s = to_select;
			printf("%d\n", s);
#else
			scanf("%d", &s);
#endif
		}

		formats[f].size.width = formats[f].sizes[s].width;
		formats[f].size.height = formats[f].sizes[s].height;
		formats[f].buffer_type = UNICAP_BUFFER_TYPE_USER;
	}

	if (!SUCCESS (unicap_set_format (handle, &formats[f]))) // (3)
	{
		fprintf(stderr, "Failed to set the format!\n");
	}
}

unicap_handle_t open_device() {
	int dev_count;
	int status = STATUS_SUCCESS;
	unicap_device_t devices[MAX_DEVICES];
	unicap_handle_t handle;
	int d = -1;

	for (dev_count = 0; SUCCESS (status) && (dev_count < MAX_DEVICES);
			dev_count++) {
		status = unicap_enumerate_devices(NULL, &devices[dev_count], dev_count); // (1)
		if (SUCCESS (status))
			printf("%d: %s\n", dev_count, devices[dev_count].identifier);
		else
			break;
	}

	if (dev_count == 0)
	{
		printf("No devices available.\n");
		return NULL; // no device selected
	}

	while ((d < 0) || (d >= dev_count)) {
		printf("Open Device: ");
#ifdef PRESELECT
		d = 0;
#else
		scanf("%d", &d);
#endif
	}

	unicap_open(&handle, &devices[d]); // (2)

	return handle;
}

void draw_box(unsigned char* array, int x, int y, int step, int size)
{
	for (int i = x - size; i < x + size; i++)
	{
		array[((y - size) * step + i) * 3] = 255;
		array[((y - size) * step + i) * 3 + 1] = 0;
		array[((y - size) * step + i) * 3 + 2] = 0;
	}
	for (int i = y - size; i < y + size; i++)
	{
		array[(i * step + x + size) * 3] = 255;
		array[(i * step + x + size) * 3 + 1] = 0;
		array[(i * step + x + size) * 3 + 2] = 0;
	}
	for (int i = x - size; i < x + size; i++)
	{
		array[((y + size) * step + i) * 3] = 255;
		array[((y + size) * step + i) * 3 + 1] = 0;
		array[((y + size) * step + i) * 3 + 2] = 0;
	}
	for (int i = y - size; i < y + size; i++)
	{
		array[(i * step + x - size) * 3] = 255;
		array[(i * step + x - size) * 3 + 1] = 0;
		array[(i * step + x - size) * 3 + 2] = 0;
	}
}

void process_frame(unicap_event_t event, unicap_handle_t handle,
		unicap_data_buffer_t *buffer, void *usr_data) {
	frameCount++;
	if ((double)frameCount / frameRate < double( clock() - start ) / (double)CLOCKS_PER_SEC)
		return;
	printf("Current time: %.2lf\n", double( clock() - start ) / (double)CLOCKS_PER_SEC);
	clock_t startTime = clock();
	volatile unicap_format_t *format = (volatile unicap_format_t *) usr_data;
	unsigned char data[format->size.width * format->size.height * 3];
	unsigned char polar[360 * (int)(sqrt(format->size.width * format->size.width + format->size.height * format->size.height) / 2) * 3];
	unsigned char result[format->size.width * format->size.height * 3];
	int count = 0;
	for (int i = 0; i < format->size.height; i++) {
		for (int j = 0; j < format->size.width; j++) {
			ucil_color_t color;
			ucil_get_pixel(buffer, &color, j, i);
//			printf("%3d ", color.yuv.y);
			data[count] = data[count + 1] = data[count + 2] = color.yuv.y;
//			int rho = (int)(sqrt((j - format->size.width / 2) * (j - format->size.width / 2) + (i - format->size.height / 2) * (i - format->size.height / 2)));
//			int theta = (int)(180 / M_PI * atan2((float)(i - format->size.height / 2), (float)(j - format->size.width / 2)));
//			polar[rho * 360 * 3 + theta * 3] = polar[rho * 360 * 3 + theta * 3 + 1] = polar[rho * 360 * 3 + theta * 3 + 2] = color.yuv.y;
			count += 3;
		}
//		printf("\n");
	}
	cout << "camera to data: " << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " seconds." << endl;
//	printf("\n");
	int size = format->size.height * format->size.width;
	int i;
	for (i = 0; i < size; i++) {
		in_img[i][0] = (double) data[i * 3];
		in_img[i][1] = 0.0;
	}
//	double omega = 2.0 * M_PI/(size - 1);
//	double A = 0.54;
//	double B = 0.46;
//	int k;
//	for(i = 0, k = 0; i < format->size.height;i++)
//	{
//	    for(int j = 0; j < format->size.width; j++, k++)
//	    {
//	        in_img[k][0]= (in_img[k][0])*(A-B*cos(omega*k));
//	        in_tpl[k][0]= (in_tpl[k][0])*(A-B*cos(omega*k));
//	    }
//	}
	fftw_execute(plan_fwd_img);
	cout << "fft: " << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " seconds." << endl;
	for (i = 0; i < size; i++) {
		// Set FFT values
//		double real = in_img[i][0] / (double) size;
//		double imag = in_img[i][1] / (double) size;
//		double mag = sqrt((real * real) + (imag * imag));
//		result[i * 3] = 10000 * mag;
//		result[i * 3 + 1] = 10000 * mag;
//		result[i * 3 + 2] = 10000 * mag;
		// Calculate cross power spectrum for each dimension (image multiplied by complex conjugate of template)
		out_fft[i][0] = (double)(out_img[i][0] * out_tpl[i][0]
				- out_img[i][1] * (-out_tpl[i][1]));
		out_fft[i][1] = (double)(out_img[i][0] * (-out_tpl[i][1])
				+ out_img[i][1] * out_tpl[i][0]);
		double tmp = sqrt(pow(out_fft[i][0], 2.0) + pow(out_fft[i][1], 2.0));
//		printf("%lf %lf %lf\n", out_tpl[i][0], out_fft[i][0] / tmp, 22tmp);
		out_fft[i][0] /= tmp;
		out_fft[i][1] /= tmp;
	}
	cout << "before ifft: " << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " seconds." << endl;
	fftw_execute(plan_bwd);
	cout << "ifft: " << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " seconds." << endl;
	double res = 0;
	double max = numeric_limits<double>::min(), min = numeric_limits<double>::max();
	int x, y;
	for (i = 0; i < size; i++) {
//		double real = out[i][0] / (double) size;
//		double imag = out[i][1] / (double) size;
//		double mag = sqrt((real * real) + (imag * imag));
//		result[i * 3] = 10000 * mag;
//		result[i * 3 + 1] = 10000 * mag;
//		result[i * 3 + 2] = 10000 * mag;
		if (res < out[i][0])
		{
			res = out[i][0];
			x = i % format->size.width;
			y = (i - x) / format->size.width;
		}

		if (max < out[i][0] / (double)size)
		{
			max = out[i][0] / (double)size;
		}
		if (min > out[i][0] / (double)size)
		{
			min = out[i][0] / (double)size;
		}
	}
#ifdef GUI
	for (i = 0; i < size; i++) {
		result[i * 3] = (char)(out[i][0] / (double)size * 255 / (max - min) - min * 255 / (max - min));
		result[i * 3 + 1] = (char)(out[i][0] / (double)size * 255 / (max - min) - min * 255 / (max - min));
		result[i * 3 + 2] = (char)(out[i][0] / (double)size * 255 / (max - min) - min * 255 / (max - min));
	}
#endif
	NetBuffer buff;
	buff.Write((char)MessageType::AUTO_AIM_DATA);
	buff.Write((int)x);
	buff.Write((int)y);
	buff.Write(res);
	ns->SendToAll(&buff, Network::NetChannel::NET_UNRELIABLE_SEQUENCED, 3);
	cout << "copy results: " << double( clock() - startTime ) / (double)CLOCKS_PER_SEC<< " seconds." << endl;
	printf("%lf %d %d\n", res, x, y);
#ifdef GUI
	draw_box(result, x, y, format->size.width, 5);
	draw_box(data, x, y, format->size.width, 5);
	/* data: original image
	 * tpl: template image
	 * result: result
	 */
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data(result, GDK_COLORSPACE_RGB,
			FALSE, 8, format->size.width, format->size.height,
			format->size.width * 3, NULL, NULL);
	GdkPixbuf *orig = //gdk_pixbuf_new_from_data(polar, GDK_COLORSPACE_RGB, FALSE,
//			8, 360, (int)(sqrt(format->size.width * format->size.width + format->size.height * format->size.height) / 2), 360 * 3,
//			NULL, NULL);
			gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB, FALSE,
			8, format->size.width, format->size.height, format->size.width * 3,
			NULL, NULL);

	gdk_threads_enter();
	gtk_image_set_from_pixbuf((GtkImage*) image, pixbuf);
	gtk_image_set_from_pixbuf((GtkImage*) image2, orig);
	g_object_unref(pixbuf);
	g_object_unref(orig);
	gtk_widget_queue_draw(image);
	gtk_widget_queue_draw(image2);
	gdk_flush ();
	gdk_threads_leave();
#endif
	printf("This frame took %.2lf seconds\n", double( clock() - startTime) / (double)CLOCKS_PER_SEC);
}

#ifdef USE_USER_BUFFERS
void capture_frames(unicap_handle_t handle) {
	unicap_format_t format;
	unicap_data_buffer_t buffer;

	if (!SUCCESS( unicap_get_format( handle, &format ) )) {
		fprintf(stderr, "Failed to get video format!\n");
	}

	format.buffer_type = UNICAP_BUFFER_TYPE_USER; // (1)

	if (!SUCCESS( unicap_set_format( handle, &format ) )) {
		fprintf(stderr, "Failed to set video format!\n");
	}

	buffer.data = (unsigned char*) malloc(format.buffer_size); // (2)
	buffer.buffer_size = format.buffer_size;

	if (!SUCCESS( unicap_start_capture(handle) )) {
		fprintf(stderr, "Failed to start video capture\n");
	}
	unicap_queue_buffer(handle, &buffer); // (4)

	while (true) {
		int count;
		unicap_poll_buffer(handle, &count);
		cout << count << endl;
		unicap_data_buffer_t* returned_buffer;
		if (!SUCCESS( unicap_wait_buffer( handle, &returned_buffer ) )) {
			fprintf(stderr, "Failed to wait for buffer!\n");
		}

		process_frame((unicap_event_t)NULL, NULL, returned_buffer, (void*)&format);

		if (!SUCCESS( unicap_queue_buffer( handle, &buffer ) )) {
			fprintf(stderr, "Failed to queue buffer!\n");
		}
	}
	unicap_stop_capture(handle);

	free(buffer.data);
}
#else
void capture_frames(unicap_handle_t handle) {
	unicap_format_t format;

	if (!SUCCESS( unicap_get_format( handle, &format ) )) {
		fprintf(stderr, "Failed to get video format!\n");
	}

	format.buffer_type = UNICAP_BUFFER_TYPE_SYSTEM; // (1)

	if (!SUCCESS( unicap_set_format( handle, &format ) )) {
		fprintf(stderr, "Failed to set video format!\n");
	}
	unicap_register_callback(handle, UNICAP_EVENT_NEW_FRAME,
			(unicap_callback_t) process_frame, (void*) &format); // (2)

	unicap_start_capture(handle); // (3)

	while (true) // (4)
	{
		usleep(1);
	}

	unicap_stop_capture(handle); // (5)

}
#endif
#ifdef CAMERA_DEBUG
static void format_change_cb(GtkWidget * ugtk, unicap_format_t * format,
		struct widgets *widgets) {
	unicapgtk_video_display_set_format(UNICAPGTK_VIDEO_DISPLAY
			(widgets->video_display), format);
}

static void device_change_cb(UnicapgtkDeviceSelection * selection,
		gchar * device_id, struct widgets *widgets) {
	unicap_device_t device;
	unicap_handle_t handle;

	unicap_void_device(&device);
	strcpy(device.identifier, device_id);

	if (!SUCCESS (unicap_enumerate_devices (&device, &device, 0))
			||
			!SUCCESS (unicap_open (&handle, &device))) {
		// device is not available anymore
		g_printerr("device '%s' not available!\n", device_id);
		return;
	}

	unicapgtk_video_format_selection_set_handle(
			UNICAPGTK_VIDEO_FORMAT_SELECTION (widgets->format_selection),
			handle);

	set_format(handle);
	ifstream fin("config.txt");
	if (!fin.is_open()) {
		printf("Failed to load configuration file\n");
	}
	printf("Loaded properties:\n");
	char property[256];
	double value;
	while (fin.get(property, 256, ':')) {
		fin.ignore(2);
		fin >> value;
		printf("%s: %.1lf\n", property, value);
		if (!SUCCESS( unicap_set_property_value( handle, property, value ) )) // (4)
		{
			fprintf(stderr, "Failed to set property!\n");
		}
		fin.getline(property, 256);
	}
	unicapgtk_video_display_stop(UNICAPGTK_VIDEO_DISPLAY
			(widgets->video_display));
	unicapgtk_video_display_set_handle(UNICAPGTK_VIDEO_DISPLAY
			(widgets->video_display), handle);
	unicapgtk_video_display_start(UNICAPGTK_VIDEO_DISPLAY
			(widgets->video_display));

	unicapgtk_property_dialog_set_handle(
			UNICAPGTK_PROPERTY_DIALOG (widgets->property_dialog), handle); // (2)
}
#endif

void startThread() {
#ifdef GUI
	gtk_main();
#endif
//	ucil_close_video_file(vobj);
}

int main(int argc, char **argv) {
#ifdef GUI
	setenv("DISPLAY", ":0", 0);
#endif
	setvbuf(stdout, NULL, _IONBF, 0);
#ifdef CAMERA_DEBUG
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *device_selection;
	struct widgets widgets;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit),
			NULL);

	vbox = gtk_vbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	hbox = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);

	widgets.video_display = unicapgtk_video_display_new();
	widgets.format_selection = unicapgtk_video_format_selection_new();
	widgets.property_dialog = unicapgtk_property_dialog_new(); // (1)

	device_selection = unicapgtk_device_selection_new(TRUE);
	gtk_box_pack_start_defaults(GTK_BOX(hbox), device_selection);
	gtk_box_pack_start_defaults(GTK_BOX(hbox), widgets.format_selection);
	unicapgtk_device_selection_rescan(UNICAPGTK_DEVICE_SELECTION
			(device_selection));
	g_signal_connect (G_OBJECT (device_selection),
			"unicapgtk_device_selection_changed",
			G_CALLBACK (device_change_cb), &widgets);
	gtk_combo_box_set_active(GTK_COMBO_BOX(device_selection), 0);

	g_signal_connect (G_OBJECT (widgets.format_selection),
			"unicapgtk_video_format_changed",
			G_CALLBACK (format_change_cb), &widgets);

	gtk_box_pack_start_defaults(GTK_BOX(vbox), widgets.video_display);

	gtk_widget_show_all(window);
	gtk_widget_show_all(widgets.property_dialog);
	gtk_main();
#else
	ns = new NetServer(8000);
	ns->Open();
	unicap_handle_t handle;
	handle = open_device();
	if (!handle)
		return -1;
	set_format(handle);
	ifstream fin("config.txt");
	if (!fin.is_open()) {
		printf("Failed to load configuration file\n");
	}
	printf("Loaded properties:\n");
	char property[256];
	double value;
	while (fin.get(property, 256, ':')) {
		fin.ignore(2);
		fin >> value;
		printf("%s: %.1lf\n", property, value);
		if (!SUCCESS( unicap_set_property_value( handle, property, value ) )) // (4)
		{
			fprintf(stderr, "Failed to set property!\n");
		}
		fin.getline(property, 256);
	}
	unicap_get_property_value(handle, "frame rate", &frameRate);
	unicap_format_t format;
	unicap_get_format(handle, &format);
//	vobj = ucil_create_video_file("test.mkv", &format, "mkv/vp8");
//	fftw_init_threads();
//	fftw_plan_with_nthreads(4);
	int size = format.size.height * format.size.width;
	in_img = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * size);
	in_tpl = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * size);
	out_fft = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * size);
	out_img = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * size);
	out_tpl = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * size);
	out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * size);
	plan_fwd_img = fftw_plan_dft_2d(format.size.height, format.size.width,
			in_img, out_img, FFTW_FORWARD, FFTW_MEASURE);
	plan_fwd_tpl = fftw_plan_dft_2d(format.size.height, format.size.width,
			in_tpl, out_tpl, FFTW_FORWARD, FFTW_MEASURE);
	plan_bwd = fftw_plan_dft_2d(format.size.height, format.size.width,
			out_fft, out, FFTW_BACKWARD, FFTW_MEASURE);
#ifdef GUI
	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_modal(GTK_WINDOW(window), true);
	g_signal_connect(G_OBJECT (window), "destroy", G_CALLBACK (gtk_main_quit),
			NULL);
	GtkWidget* layout = gtk_fixed_new();
	image = gtk_image_new();
	image2 = gtk_image_new();
//	video = unicapgtk_video_display_new();
//	unicapgtk_video_display_set_handle(UNICAPGTK_VIDEO_DISPLAY(video), handle);
//	unicapgtk_video_display_start(UNICAPGTK_VIDEO_DISPLAY(video));
//	gtk_box_pack_start_defaults(GTK_BOX(vbox), video);
	gtk_fixed_put(GTK_FIXED(layout), image, 0, 0);
	gtk_fixed_put(GTK_FIXED(layout), image2, 640, 0);
	gtk_container_add(GTK_CONTAINER(window), layout);
	gtk_widget_show_all(window);
#endif
#ifdef GUI
	GError** error = NULL;
	tpl_pix = gdk_pixbuf_new_from_file(TEMPLATE_FILE, error);
	tpl_pix2 = gdk_pixbuf_new_from_file("target.jpg", error);
	tpl = (unsigned char*) malloc(
			gdk_pixbuf_get_width(tpl_pix) * gdk_pixbuf_get_height(tpl_pix) * 3);
	tpl = (unsigned char*) gdk_pixbuf_get_pixels(tpl_pix);
	tpl2 = (unsigned char*) malloc(
			gdk_pixbuf_get_width(tpl_pix2) * gdk_pixbuf_get_height(tpl_pix2)
					* 3);
	tpl2 = (unsigned char*) gdk_pixbuf_get_pixels(tpl_pix2);
#else
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	FILE * infile;
	if ((infile = fopen(TEMPLATE_FILE, "rb")) == NULL) {
		fprintf(stderr, "Can't open template file\n");
	}
	jpeg_stdio_src(&cinfo, infile);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	tpl = (unsigned char*) malloc(cinfo.image_width  * cinfo.image_height * 3);
	int row_stride = cinfo.image_width * 3;
	while (cinfo.output_scanline < cinfo.image_height)
	{
		jpeg_read_scanlines(&cinfo, &tpl, 1);
		tpl += row_stride;
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	jpeg_abort_decompress(&cinfo);
	tpl -= cinfo.image_width  * cinfo.image_height * 3;
#endif
	int i;
	for (i = 0; i < size; i++) {
		in_tpl[i][0] = (double) tpl[i * 3];
		in_tpl[i][1] = 0.0;
	}
	fftw_execute(plan_fwd_tpl);
#ifdef GUI
	gdk_threads_enter();
	boost::thread* m_task;
	m_task = new boost::thread(startThread);
	gdk_threads_leave();
#endif
	frameCount = 0;
	start = clock();
	capture_frames(handle);
	unicap_close(handle); // (3)
	fftw_destroy_plan(plan_fwd_img);
	fftw_destroy_plan(plan_fwd_tpl);
	fftw_destroy_plan(plan_bwd);
	fftw_free(in_img);
	fftw_free(in_tpl);
	fftw_free(out_fft);
	fftw_free(out_img);
	fftw_free(out_tpl);
	fftw_free(out);
	fftw_cleanup_threads();
#ifdef GUI
	m_task->interrupt();
#endif
	return 0;
#endif

	return 0;
}
